#include "Weapons/CWeapon.h"

#include "Commons/CYJJBlueprintLibrary.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CTargetingComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Components/CWeaponComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utilities/CLog.h"
#include "Weapons/CAttachment.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACWeapon::ACWeapon()
{
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	PrimaryActorTick.bCanEverTick = true;
}

void ACWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 레거시 ExecutionSequence 순서(Set Character→Set Component→Load Equip→Load DoAction→Load Hit).
	Weapon_SetCharacterAndComponentsFromBlueprint();
	Weapon_ClearRuntimeTableCaches();
	Weapon_LoadEquipDatasFromTable();
	Weapon_LoadDoActionDatasFromTables();
	Weapon_LoadHitDatasFromTable();
}

void ACWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Tick 경로에서는 Owner/상태 검사 빈번 — 로깅하면 스팸이므로 주석만 둔다.
	if (false == IsValid(Character))
		return;

	UCStateComponent* StateComp = Character->FindComponentByClass<UCStateComponent>();

	if (IsValid(StateComp) && StateComp->IsRealRiding())
	{
		AActor* InteractingActor = Character->CurInteractingActor;
		if (IsValid(InteractingActor))
		{
			UCMovementComponent* MoveCompFromInteractor =
				InteractingActor->FindComponentByClass<UCMovementComponent>();
			if (IsValid(MoveCompFromInteractor))
				Moving = MoveCompFromInteractor;
		}
		// 레거시: CurInteractingActor 비유효 시 Moving 갱신 핀 미연결 — 이전 Tick 값 유지.

		return;
	}

	Moving = Character->FindComponentByClass<UCMovementComponent>();
}

void ACWeapon::PlayAction(
	const TArray<FDoActionData>& InDatas,
	const int32 InIndex,
	const FVector& InHitPoint)
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] PlayAction(Character 없음): %s Owner=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetOwner())));
		return;
	}

	if (false == InDatas.IsValidIndex(InIndex))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] PlayAction(인덱스 범위 밖 — %s): Index=%d Count=%d"),
			*GetNameSafe(this),
			InIndex,
			InDatas.Num()));
		return;
	}

	const FDoActionData& Row = InDatas[InIndex];
	ACPlayableCharacter* PlayableActor = Cast<ACPlayableCharacter>(Character);

	// 레거시: 플레이어 캐스트 성공 시에만 ConsumeStamina 시도 후 실패하면 End Do Action 처리.
	if (IsValid(PlayableActor))
	{
		const bool bEnoughStamina =
			PlayableActor->ConsumeStamina(static_cast<double>(Row.Stamina));
		if (false == bEnoughStamina)
		{
			End_DoAction(PrevAttackType);
			return;
		}
	}

	Character->PlayAnimMontage(Row.Montage, Row.PlayRate);

	Weapon_PlayActionRunExecutionBranches(Row, InHitPoint, PlayableActor);
}

void ACWeapon::End_DoAction(CEAttackType InAttackType)
{
	// 레거시 Weapon_End_DoAction 그래프: InAction 해제 → Move/UnFixCamera → RestoreState 이후 활성 UCAct 종료.
	InAction = false;

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] End_DoAction(Character 없음 — 이동·복구 생략): %s attack=%d"),
			*GetNameSafe(this),
			static_cast<int32>(InAttackType)));
		return;
	}

	if (IsValid(Moving))
	{
		Moving->Move();
		Moving->UnFixCamera();
	}

	Character->ApplyRestoreStateFromPrevMode();

	if (UCWeaponComponent* WeaponComp = Character->FindComponentByClass<UCWeaponComponent>())
	{
		WeaponComp->End_DoAction(InAttackType);
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] End_DoAction: UCWeaponComponent 없음 — Character=%s"),
			*GetNameSafe(Character)));
	}
}

void ACWeapon::RestorePrevState()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] RestorePrevState(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	Character->ApplyRestoreStateFromPrevMode();
}

bool ACWeapon::ConsumeStamina(const double InStamina)
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] ConsumeStamina(Character 없음): %s"),
			*GetNameSafe(this)));
		return false;
	}

	ACPlayableCharacter* PlayableActor = Cast<ACPlayableCharacter>(Character);
	if (false == IsValid(PlayableActor))
	{
		return true;
	}

	return PlayableActor->ConsumeStamina(InStamina);
}

void ACWeapon::Destroy_FromWeaponBlueprint()
{
	Destroy();
}

void ACWeapon::DoAction(const CEAttackType InAttackType, const int32 InSkillIndex)
{
	Weapon_DoActionImpl(InAttackType, InSkillIndex);
}

void ACWeapon::Weapon_DoActionImpl(const CEAttackType InAttackType, const int32 InSkillIndex)
{
	(void)InSkillIndex;

	CancelDashes();

	if (true == InAction && InAttackType != PrevAttackType)
	{
		const CEAttackType PrevForEnd = PrevAttackType;
		End_DoAction(PrevForEnd);
		PrevAttackType = InAttackType;
		InAction = true;
		return;
	}

	InAction = true;
}

void ACWeapon::Begin_DoAction_Implementation(const CEAttackType InAttackType)
{
	(void)InAttackType;
}

UCStateComponent* ACWeapon::Weapon_ResolveStateComponent() const
{
	if (false == IsValid(Character))
		return nullptr;

	UCStateComponent* StateComp = Cast<UCStateComponent>(State);
	if (IsValid(StateComp))
		return StateComp;

	return Character->FindComponentByClass<UCStateComponent>();
}

bool ACWeapon::Weapon_ShouldAirDashFromMovementState() const
{
	const UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (IsValid(StateComp))
	{
		if (StateComp->IsFalling() || StateComp->IsFlying())
			return true;
	}

	ACharacter* Ch = Cast<ACharacter>(Character);
	if (false == IsValid(Ch))
		return false;

	UCharacterMovementComponent* MoveComp = Ch->GetCharacterMovement();
	if (false == IsValid(MoveComp))
		return false;

	return MoveComp->IsFalling() || MoveComp->IsFlying();
}

void ACWeapon::Pressed_Implementation()
{
	CancelDashes();
}

void ACWeapon::Released_Implementation()
{
}

void ACWeapon::Hold_SubWeapon_Implementation()
{
}

void ACWeapon::Released_SubWeapon_Implementation()
{
}

void ACWeapon::DoSubWeaponAction_Implementation()
{
	CancelDashes();
}

void ACWeapon::Begin_DoSubWeaponAction_Implementation()
{
	InSubWeaponAction = true;
}

void ACWeapon::End_DoSubWeaponAction_Implementation()
{
	InSubWeaponAction = false;
}

void ACWeapon::Dash()
{
	if (DoMovingActionDatas.Num() <= 0)
		return;

	if (true == InGroundDashing || true == InAirDashing)
	{
		return;
	}

	if (true == InAction)
	{
		End_DoAction(CEAttackType::Common);
	}

	if (true == Weapon_ShouldAirDashFromMovementState())
	{
		AirDash();
	}
	else
	{
		GroundDash();
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (IsValid(StateComp))
	{
		StateComp->SetDash();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] Dash: UCStateComponent 없어 SetDash 생략 — %s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
	}
}

void ACWeapon::GroundDash()
{
	InGroundDashing = true;
	InAirDashing = false;
	PlayAction(DoMovingActionDatas, GroundDashIndex, FVector::ZeroVector);
}

void ACWeapon::End_GroundDash()
{
	InGroundDashing = false;
	InAirDashing = false;
	RestorePrevState();
}

void ACWeapon::AirDash()
{
	UCMovementComponent* MoveRef = Moving;
	if (false == IsValid(MoveRef) && IsValid(Character))
	{
		MoveRef = Character->FindComponentByClass<UCMovementComponent>();
	}

	if (false == IsValid(MoveRef))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] AirDash: Moving/UCMovementComponent 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	MoveRef->SetLerpMove(true);
	InAirDashing = true;
	InGroundDashing = false;
	PlayAction(DoMovingActionDatas, AirDashIndex, FVector::ZeroVector);
}

void ACWeapon::End_AirDash()
{
	InAirDashing = false;
	InGroundDashing = false;

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] End_AirDash(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	ACPlayableCharacter* Playable = Cast<ACPlayableCharacter>(Character);
	if (false == IsValid(Playable))
	{
		return;
	}

	Playable->StartFall(2.0);
	RestorePrevState();
}

void ACWeapon::CancelDashes()
{
	if (true == InGroundDashing)
	{
		End_GroundDash();
	}

	if (true == InAirDashing)
	{
		End_AirDash();
	}
}

void ACWeapon::SetControllerInEquip()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] SetControllerInEquip(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	Controller = Character->CurController.Get();
}

void ACWeapon::Equip_Implementation()
{
	CancelDashes();

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] Equip(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	SetControllerInEquip();

	UCStateComponent* StateComp = Cast<UCStateComponent>(State);
	if (false == IsValid(StateComp))
		StateComp = Character->FindComponentByClass<UCStateComponent>();

	if (IsValid(StateComp))
	{
		StateComp->SetEquip();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] Equip: UCStateComponent 없음 — Character=%s 무기=%s"),
			*GetNameSafe(Character),
			*GetNameSafe(this)));
	}

	Weapon_ApplyEquipRowFacingAndMovement(EquipData);

	if (IsValid(EquipData.Montage))
	{
		Character->PlayAnimMontage(EquipData.Montage, EquipData.PlayRate);
		return;
	}

	Begin_Equip(true);
	End_Equip(true);
}

void ACWeapon::Unequip_Implementation()
{
	CancelDashes();

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] Unequip(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	InEquip = false;

	if (IsValid(Moving))
	{
		Moving->DisableControlRotation();

		if (UnequipData.bCanMove)
			Moving->Move();
		else
			Moving->Stop();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] Unequip: Moving 미연결 — %s"),
			*GetNameSafe(this)));
	}

	if (IsValid(UnequipData.Montage))
	{
		Character->PlayAnimMontage(UnequipData.Montage, UnequipData.PlayRate);
		return;
	}

	Begin_Unequip(true);
	End_Unequip(true);
}

void ACWeapon::Begin_Equip_Implementation(bool bMainOrSubWeapon)
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] Begin_Equip(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	if (bMainOrSubWeapon)
		IsEquipping = true;

	if (IsValid(EquipData.Sound))
		EquipData.PlaySoundWave(TWeakObjectPtr<ACCommonCharacter>(Character));
}

void ACWeapon::End_Equip_Implementation(bool bMainOrSubWeapon)
{
	if (false == bMainOrSubWeapon)
		return;

	IsEquipping = false;
	InEquip = true;

	if (IsValid(Moving))
		Moving->Move();

	RestorePrevState();
}

void ACWeapon::Begin_Unequip_Implementation(bool bMainOrSubWeapon)
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] Begin_Unequip(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	if (bMainOrSubWeapon)
		IsUnequipping = true;

	if (bMainOrSubWeapon)
	{
		if (IsValid(UnequipData.Sound))
			UnequipData.PlaySoundWave(TWeakObjectPtr<ACCommonCharacter>(Character));
	}
	else
	{
		if (IsValid(EquipData.Sound))
			EquipData.PlaySoundWave(TWeakObjectPtr<ACCommonCharacter>(Character));
	}
}

void ACWeapon::End_Unequip_Implementation(bool bMainOrSubWeapon)
{
	if (false == bMainOrSubWeapon)
		return;

	IsUnequipping = false;
	InEquip = false;

	if (IsValid(Moving))
		Moving->Move();
}

void ACWeapon::Weapon_ApplyEquipRowFacingAndMovement(const FEquipData& Row)
{
	if (false == IsValid(Moving))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] Equip 이동 분기 중 Moving 미연결 — %s"),
			*GetNameSafe(this)));
		return;
	}

	if (Row.bUseControlRotation)
		Moving->EnableControlRotation();
	else
		Moving->DisableControlRotation();

	if (Row.bCanMove)
		Moving->Move();
	else
		Moving->Stop();
}

void ACWeapon::Weapon_PlayActionRunExecutionBranches(
	const FDoActionData& InRow,
	const FVector& InHitPoint,
	ACPlayableCharacter* OptionalPlayer)
{
	// 레거시 ExecutionSequence 순서와 동일: Move/Stop → FixCamera 여부 → 이펙트·사운드 → 모션 트레일 → 런치.
	if (IsValid(Moving))
	{
		if (InRow.bCanMove)
			Moving->Move();
		else
			Moving->Stop();

		if (InRow.bFixedCamera)
			Moving->FixCamera();
	}
	else if (InRow.bCanMove || InRow.bFixedCamera)
		CLog::Log(FString::Printf(
			TEXT("[Weapon] PlayAction: Moving 무효 — Move/카메라 분기 생략 — %s"),
			*GetNameSafe(this)));

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		const TWeakObjectPtr<UWorld> WorldWeak(World);
		if (IsValid(InRow.Effect))
			InRow.PlayEffect(WorldWeak, InHitPoint);

		if (IsValid(InRow.Sound))
			UGameplayStatics::PlaySoundAtLocation(World, InRow.Sound, InHitPoint);
	}

	if (true == InRow.bUseMotionTrail && IsValid(OptionalPlayer))
		OptionalPlayer->SetVisibleMotionTrail();

	if ((false == FMath::IsNearlyZero(InRow.LaunchForward)) || (false == FMath::IsNearlyZero(InRow.LaunchUp)))
	{
		ACharacter* CharacterLauncher = Cast<ACharacter>(Character);
		if (false == IsValid(CharacterLauncher))
			return;

		USkeletalMeshComponent* Mesh = CharacterLauncher->GetMesh();
		if (false == IsValid(Mesh))
			return;

		const FVector RightScaled =
			UKismetMathLibrary::Multiply_VectorFloat(Mesh->GetRightVector(), InRow.LaunchForward);
		const FVector UpScaled = UKismetMathLibrary::Multiply_VectorFloat(Mesh->GetUpVector(), InRow.LaunchUp);
		// BP LaunchCharacter 분할 핀: XY 는 (Right×LaunchForward), Z 는 (Up×LaunchUp) 의 Z 성분 조합과 동등에 가깝게 맞춤.
		const FVector LaunchVelocity(
			RightScaled.X,
			RightScaled.Y,
			UpScaled.Z);

		CharacterLauncher->LaunchCharacter(LaunchVelocity, false, false);
	}
}

void ACWeapon::Weapon_SetCharacterAndComponentsFromBlueprint()
{
	Character = Cast<ACCommonCharacter>(GetOwner());

	if (false == IsValid(Character))
	{
		Moving = nullptr;
		State = nullptr;
		// 무기 칠드액터에 Owner 미달 시 이후 표 로드만 시도된다 — 디버깅 시 추적할 수 있게 한 번 로그한다.
		CLog::Log(FString::Printf(
			TEXT("[Weapon] BeginPlay(Set Character 실패): Owner 가 ACCommonCharacter 가 아니거나 무효입니다. Actor=%s Owner=%s"),
			*GetName(),
			IsValid(GetOwner()) ? *GetOwner()->GetName() : TEXT("(null)")));
		return;
	}

	Moving = Character->FindComponentByClass<UCMovementComponent>();
	if (false == IsValid(Moving))
		CLog::Log(FString::Printf(
			TEXT("[Weapon] 캐릭터 %s 에 UCMovementComponent 없음 — Moving 미연결."), *Character->GetName()));

	State = Character->FindComponentByClass<UCStateComponent>();
	// 상태 컴포넌트 미부착 캐릭터 클래스도 있을 수 있어 여기선 침묵한다.
}

void ACWeapon::Weapon_ClearRuntimeTableCaches()
{
	DoActionDatas.Reset();
	DoAirComboDatas.Reset();
	DoFlyingAttackDatas.Reset();
	DoDownAttackDatas.Reset();
	DoSwayingAttackDatas.Reset();
	DoFallDownAttackDatas.Reset();
	SkillDatas.Reset();
	DoRidingAttackDatas.Reset();
	DoMovingActionDatas.Reset();

	HitCommonDatas.Reset();
	HitAirComboDatas.Reset();
	HitFlyingAttackDatas.Reset();
	HitDownAttackDatas.Reset();
	HitSwayingAttackDatas.Reset();
	HitFallDownAttackDatas.Reset();
	HitSkillDatas.Reset();
	HitRidingAttackDatas.Reset();
}

void ACWeapon::Weapon_LoadEquipDatasFromTable()
{
	if (false == IsValid(EquipDataTable))
		return;

	const TArray<FName> RowNames = EquipDataTable->GetRowNames();
	const int32 RowCount = RowNames.Num();

	// 레거시: 순서 브랜치 — 첫 줄 Common/장착, 둘째 Unequip 라는 가정(행 이름 순서 의존).
	if (RowCount > 0)
	{
		if (const FEquipData* EquipRow = EquipDataTable->FindRow<FEquipData>(RowNames[0], TEXT("WeaponEquip_Row0")))
			EquipData = *EquipRow;
		else
			CLog::Log(FString::Printf(
				TEXT("[Weapon] EquipDataTable '%s' 첫 행 '%s' 를 FEquipData 로 읽지 못했습니다."),
				*EquipDataTable->GetName(),
				*RowNames[0].ToString()));
	}

	if (RowCount > 1)
	{
		if (const FEquipData* UnequipRow =
				EquipDataTable->FindRow<FEquipData>(RowNames[1], TEXT("WeaponEquip_Row1")))
			UnequipData = *UnequipRow;
		else
			CLog::Log(FString::Printf(
				TEXT("[Weapon] EquipDataTable '%s' 둘째 행 '%s' 를 FEquipData 로 읽지 못했습니다."),
				*EquipDataTable->GetName(),
				*RowNames[1].ToString()));
	}
}

void ACWeapon::Weapon_AppendDoActionRowByAttackType(
	ACWeapon* InWeapon,
	const FDoActionData& InRow)
{
	if (false == IsValid(InWeapon))
		return;

	switch (InRow.AttackType)
	{
	case CEAttackType::Common:
		InWeapon->DoActionDatas.Add(InRow);
		break;
	case CEAttackType::Air:
		InWeapon->DoAirComboDatas.Add(InRow);
		break;
	case CEAttackType::Flying:
		InWeapon->DoFlyingAttackDatas.Add(InRow);
		break;
	case CEAttackType::Down:
		InWeapon->DoDownAttackDatas.Add(InRow);
		break;
	case CEAttackType::DashAttack:
		InWeapon->DoSwayingAttackDatas.Add(InRow);
		break;
	case CEAttackType::FallDown:
		InWeapon->DoFallDownAttackDatas.Add(InRow);
		break;
	case CEAttackType::Dash:
	case CEAttackType::AirDash:
		// 레거시 메인 DoAction 표는 무시하고 DoMovingActionDataTable 만 적재 경로가 있다.
		break;
	case CEAttackType::Skill:
		InWeapon->SkillDatas.Add(InRow);
		break;
	case CEAttackType::Riding:
		InWeapon->DoRidingAttackDatas.Add(InRow);
		break;
	case CEAttackType::Max:
	default:
		break;
	}
}

void ACWeapon::Weapon_AppendMovingDoActionRowIfDashTypes(
	ACWeapon* InWeapon,
	const FDoActionData& InRow)
{
	if (false == IsValid(InWeapon))
		return;

	switch (InRow.AttackType)
	{
	case CEAttackType::Dash:
	case CEAttackType::AirDash:
		InWeapon->DoMovingActionDatas.Add(InRow);
		break;
	default:
		break;
	}
}

void ACWeapon::Weapon_LoadDoActionDatasFromTables()
{
	if (IsValid(DoActionDataTable))
	{
		const TArray<FName> RowNames = DoActionDataTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			if (const FDoActionData* Row =
					DoActionDataTable->FindRow<FDoActionData>(RowName, TEXT("WeaponDoAction_Main")))
				Weapon_AppendDoActionRowByAttackType(this, *Row);
		}
	}

	if (IsValid(DoMovingActionDataTable))
	{
		const TArray<FName> RowNames = DoMovingActionDataTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			if (const FDoActionData* Row =
					DoMovingActionDataTable->FindRow<FDoActionData>(RowName, TEXT("WeaponDoAction_Move")))
				Weapon_AppendMovingDoActionRowIfDashTypes(this, *Row);
		}
	}
}

void ACWeapon::Weapon_AppendHitRowByAttackType(
	ACWeapon* InWeapon,
	const FHitData& InRow)
{
	if (false == IsValid(InWeapon))
		return;

	switch (InRow.AttackType)
	{
	case CEAttackType::Common:
		InWeapon->HitCommonDatas.Add(InRow);
		break;
	case CEAttackType::Air:
		InWeapon->HitAirComboDatas.Add(InRow);
		break;
	case CEAttackType::Flying:
		InWeapon->HitFlyingAttackDatas.Add(InRow);
		break;
	case CEAttackType::Down:
		InWeapon->HitDownAttackDatas.Add(InRow);
		break;
	case CEAttackType::DashAttack:
		InWeapon->HitSwayingAttackDatas.Add(InRow);
		break;
	case CEAttackType::FallDown:
		InWeapon->HitFallDownAttackDatas.Add(InRow);
		break;
	case CEAttackType::Skill:
		InWeapon->HitSkillDatas.Add(InRow);
		break;
	default:
		// 레거시 Load Hit 피처 전환에서 Dash/AirDash/Riding 실행 핀 미연결 — 행 자체가 이런 타입이면 여기까지 오면 무시된다.
		break;
	}
}

void ACWeapon::Weapon_LoadHitDatasFromTable()
{
	if (false == IsValid(HitDataTable))
		return;

	const TArray<FName> RowNames = HitDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		if (const FHitData* Row = HitDataTable->FindRow<FHitData>(RowName, TEXT("WeaponHit")))
			Weapon_AppendHitRowByAttackType(this, *Row);
	}
}

void ACWeapon::Host_DoAction(const CEAttackType InAttackType, const int32 InSkillIndex)
{
	DoAction(InAttackType, InSkillIndex);
}

void ACWeapon::Host_BeginDoAction(const CEAttackType InAttackType)
{
	Begin_DoAction(InAttackType);
}

void ACWeapon::Host_EndDoAction(const CEAttackType InAttackType)
{
	End_DoAction(InAttackType);
}
