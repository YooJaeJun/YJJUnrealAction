#include "Weapons/CSkillWeapon.h"

#include "Commons/CYJJBlueprintLibrary.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/ShapeComponent.h"
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
#include "GameFramework/Controller.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utilities/CLog.h"
#include "Weapons/CAttachment.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	constexpr float ComboTickTargetingInterpSpeed = 5.0f;
	constexpr float ComboTickTargetingRotatorTolerance = 1.0f;
	constexpr float ComboTickTargetingMouseTolerance = 1.0f;
	constexpr float ComboBeginFlyingNotifyAddGravityFactor = 1.2f;
	constexpr float ComboBeginAirLaunchUp = 1000.0f;
	constexpr float ComboFallDownGravityDelaySeconds = 0.3f;
	// 레거시 `Weapon_Combo:FallDownGravity` → Moving `SetGravity(InGravity)` 핀 기본값(= `CharacterMovement::GravityScale`).
	constexpr float ComboFallDownGravityBpGravityScale = 20.f;
	// 레거시 `Weapon_Guard` Tick `ClearHitted` 브랜치 `Delay`(0.8s).
	constexpr float GuardClearHittedDelaySeconds = 0.8f;

	AController* ComboResolveControllerForWeaponContext(ACWeaponSkillContext* InWeapon)
	{
		if (false == IsValid(InWeapon))
		{
			return nullptr;
		}

		if (IsValid(InWeapon->Controller))
		{
			return InWeapon->Controller;
		}

		if (false == IsValid(InWeapon->Character))
		{
			return nullptr;
		}

		ACCommonCharacter* AsCommon = Cast<ACCommonCharacter>(InWeapon->Character);
		if (IsValid(AsCommon))
		{
			const TWeakObjectPtr<AController> Cur = AsCommon->GetMyCurController();
			if (Cur.IsValid())
			{
				return Cur.Get();
			}

			return AsCommon->GetController();
		}

		return InWeapon->Character->GetController();
	}
} // namespace

void ACMagicSkillContext::SetControllerInEquip()
{
	ACCommonCharacter* OwnerChar = Cast<ACCommonCharacter>(GetOwner());
	if (false == IsValid(OwnerChar))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] SetControllerInEquip(Owner 가 ACCommonCharacter 가 아님 — Controller 미갱신): 컨텍스트=%s Owner=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetOwner())));
		return;
	}

	// 레거시 BP: BP_Character.CurController → Magic.Controller (Character 멤버가 아닌 Owning 스택 기준으로 맞춤).
	Controller = OwnerChar->CurController.Get();
}

UCStateComponent* ACMagicSkillContext::Magic_ResolveStateComponent() const
{
	if (false == IsValid(Character))
	{
		return nullptr;
	}

	UCStateComponent* StateComp = Cast<UCStateComponent>(State);
	if (IsValid(StateComp))
	{
		return StateComp;
	}

	return Character->FindComponentByClass<UCStateComponent>();
}

void ACMagicSkillContext::Magic_ApplyEquipRowFacingAndMovement(const FEquipData& Row)
{
	if (false == IsValid(Moving))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] Equip 이동 분기 중 Moving 미연결 — %s"),
			*GetNameSafe(this)));
		return;
	}

	if (Row.bUseControlRotation)
	{
		Moving->EnableControlRotation();
	}
	else
	{
		Moving->DisableControlRotation();
	}

	if (Row.bCanMove)
	{
		Moving->Move();
	}
	else
	{
		Moving->Stop();
	}
}

void ACMagicSkillContext::Equip_Implementation()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] Equip(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	SetControllerInEquip();

	UCStateComponent* StateComp = Magic_ResolveStateComponent();
	if (IsValid(StateComp))
	{
		StateComp->SetEquip();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] Equip: UCStateComponent 없음 — Character=%s 무기=%s"),
			*GetNameSafe(Character),
			*GetNameSafe(this)));
	}

	Magic_ApplyEquipRowFacingAndMovement(EquipData);

	if (IsValid(EquipData.Montage))
	{
		Character->PlayAnimMontage(EquipData.Montage, EquipData.PlayRate);
		return;
	}

	// 몽타주 없을 때만 Begin/End 장착 훅(사운드 + InEquip 플래그 + RestorePrevState)을 연달아 호출 — 레거시 Equip 그래프 노드 순서와 동일.
	Begin_Equip();
	End_Equip();
}

void ACMagicSkillContext::Unequip_Implementation()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] Unequip(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	InEquip = false;

	if (IsValid(Moving))
	{
		Moving->DisableControlRotation();

		if (UnequipData.bCanMove)
		{
			Moving->Move();
		}
		else
		{
			Moving->Stop();
		}
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] Unequip: Moving 미연결 — %s"),
			*GetNameSafe(this)));
	}

	if (IsValid(UnequipData.Montage))
	{
		Character->PlayAnimMontage(UnequipData.Montage, UnequipData.PlayRate);
		return;
	}

	Begin_Unequip();
	End_Unequip();
}

void ACMagicSkillContext::Begin_Equip_Implementation()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] Begin_Equip(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	IsEquipping = true;

	if (IsValid(EquipData.Sound))
	{
		EquipData.PlaySoundWave(TWeakObjectPtr<ACCommonCharacter>(Character));
	}
}

void ACMagicSkillContext::End_Equip_Implementation()
{
	IsEquipping = false;
	InEquip = true;

	if (IsValid(Moving))
	{
		Moving->Move();
	}

	RestorePrevState();
}

void ACMagicSkillContext::Begin_Unequip_Implementation()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] Begin_Unequip(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	IsUnequipping = true;

	if (IsValid(UnequipData.Sound))
	{
		UnequipData.PlaySoundWave(TWeakObjectPtr<ACCommonCharacter>(Character));
	}
}

void ACMagicSkillContext::End_Unequip_Implementation()
{
	IsUnequipping = false;
	InEquip = false;

	if (IsValid(Moving))
	{
		Moving->Move();
	}
}

void ACMagicSkillContext::DoAction_Implementation(CEAttackType InAttackType, int32 InSkillIndex)
{
	(void)InSkillIndex;

	// 레거시 Magic DoAction 그래프: Common 이 아닌 새 액션은 진행 중이면 PrevAttackType 로 End 후 타입 교체.
	if (true == InAction && InAttackType != CEAttackType::Common)
	{
		const CEAttackType PrevForEnd = PrevAttackType;
		End_DoAction(PrevForEnd);
		PrevAttackType = InAttackType;
		InAction = true;
		return;
	}

	InAction = true;
}

void ACMagicSkillContext::Begin_DoAction_Implementation(CEAttackType InAttackType)
{
}

void ACMagicSkillContext::End_DoAction_Implementation(CEAttackType InAttackType)
{
	// 레거시 Magic End_DoAction BP 는 비었으나 애니 노티 종료 체인에서 호출된다 — 무기 End_DoAction 과 동등하게 상태·Moving 을 원복한다(UCMagicComponent 재진입 금지).
	InAction = false;

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] End_DoAction(Character 없음 — 이동·복구 생략): %s attack=%d"),
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

	(void)InAttackType;
}

ACMagicSkillContext::ACMagicSkillContext()
{
	// 레거시 Magic ReceiveTick — 탑승 시 Moving 을 상호작용 액터 쪽 컴포넌트로 갱신한다.
	PrimaryActorTick.bCanEverTick = true;
}

void ACMagicSkillContext::BeginPlay()
{
	Super::BeginPlay();

	Magic_ClearRuntimeTableCaches();
	Magic_SetCharacterAndComponentsFromBlueprint();
	Magic_LoadEquipDatasFromTable();
	Magic_LoadDoActionDatasFromTable_CommonOnly();
	Magic_LoadHitDatasFromTable_CommonOnly();
}

void ACMagicSkillContext::Magic_ClearRuntimeTableCaches()
{
	DoActionDatas.Reset();
	HitCommonDatas.Reset();
}

void ACMagicSkillContext::Magic_SetCharacterAndComponentsFromBlueprint()
{
	Character = Cast<ACCommonCharacter>(GetOwner());

	if (false == IsValid(Character))
	{
		Moving = nullptr;
		State = nullptr;
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] BeginPlay(Set Character 실패): Owner 가 ACCommonCharacter 가 아니거나 무효 — Actor=%s Owner=%s"),
			*GetName(),
			IsValid(GetOwner()) ? *GetOwner()->GetName() : TEXT("(null)")));
		return;
	}

	Moving = Character->FindComponentByClass<UCMovementComponent>();
	if (false == IsValid(Moving))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] 캐릭터 %s 에 UCMovementComponent 없음 — Moving 미연결."),
			*Character->GetName()));
	}

	State = Character->FindComponentByClass<UCStateComponent>();
}

void ACMagicSkillContext::Magic_LoadEquipDatasFromTable()
{
	if (false == IsValid(EquipDataTable))
		return;

	const TArray<FName> RowNames = EquipDataTable->GetRowNames();
	const int32 RowCount = RowNames.Num();

	if (RowCount > 0)
	{
		if (const FEquipData* EquipRow = EquipDataTable->FindRow<FEquipData>(RowNames[0], TEXT("MagicEquip_Row0")))
			EquipData = *EquipRow;
		else
			CLog::Log(FString::Printf(
				TEXT("[MagicSkillContext] EquipDataTable '%s' 첫 행 '%s' 를 FEquipData 로 읽지 못했습니다."),
				*EquipDataTable->GetName(),
				*RowNames[0].ToString()));
	}

	if (RowCount > 1)
	{
		if (const FEquipData* UnequipRow = EquipDataTable->FindRow<FEquipData>(RowNames[1], TEXT("MagicEquip_Row1")))
			UnequipData = *UnequipRow;
		else
			CLog::Log(FString::Printf(
				TEXT("[MagicSkillContext] EquipDataTable '%s' 둘째 행 '%s' 를 FEquipData 로 읽지 못했습니다."),
				*EquipDataTable->GetName(),
				*RowNames[1].ToString()));
	}
}

void ACMagicSkillContext::Magic_LoadDoActionDatasFromTable_CommonOnly()
{
	if (false == IsValid(DoActionDataTable))
		return;

	const TArray<FName> RowNames = DoActionDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		if (const FDoActionData* Row =
				DoActionDataTable->FindRow<FDoActionData>(RowName, TEXT("MagicDoAction_Common")))
		{
			if (Row->AttackType == CEAttackType::Common)
				DoActionDatas.Add(*Row);
		}
	}
}

void ACMagicSkillContext::Magic_LoadHitDatasFromTable_CommonOnly()
{
	if (false == IsValid(HitDataTable))
		return;

	const TArray<FName> RowNames = HitDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		if (const FHitData* Row = HitDataTable->FindRow<FHitData>(RowName, TEXT("MagicHit_Common")))
		{
			if (Row->AttackType == CEAttackType::Common)
				HitCommonDatas.Add(*Row);
		}
	}
}

void ACMagicSkillContext::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Tick 경로 — Owner·탑승 상태 검출이 매 프레임이라 로그 없음(무기 ACSkillWeapon::Tick 과 동형).
	(void)DeltaSeconds;

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

		return;
	}

	Moving = Character->FindComponentByClass<UCMovementComponent>();
}

void ACMagicSkillContext::PlayAction(const TArray<FDoActionData>& InDatas, int32 InIndex)
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] PlayAction(Character 없음): %s Owner=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetOwner())));
		return;
	}

	if (false == InDatas.IsValidIndex(InIndex))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] PlayAction(인덱스 범위 밖 — %s): Index=%d Count=%d"),
			*GetNameSafe(this),
			InIndex,
			InDatas.Num()));
		return;
	}

	const FDoActionData& Row = InDatas[InIndex];
	ACPlayableCharacter* PlayableActor = Cast<ACPlayableCharacter>(Character);

	// 레거시: 플레이어 캐스트 성공 시 ConsumeMana 분기만 탄다 — NPC 는 마나 차감 없이 몽타주만 재생.
	if (IsValid(PlayableActor))
	{
		const bool bEnoughMana = ConsumeMana(static_cast<double>(Row.Mana));
		if (false == bEnoughMana)
		{
			End_DoAction(PrevAttackType);
			return;
		}
	}

	Character->PlayAnimMontage(Row.Montage, Row.PlayRate);

	Magic_PlayActionRunExecutionBranches(Row, PlayableActor);
}

void ACMagicSkillContext::RestorePrevState()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] RestorePrevState(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	Character->ApplyRestoreStateFromPrevMode();
}

bool ACMagicSkillContext::ConsumeMana(const double InMana)
{
	ACPlayableCharacter* PlayableResolve = Cast<ACPlayableCharacter>(Character);

	if (false == IsValid(PlayableResolve))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicSkillContext] ConsumeMana(플레이어가 아님 — 실패 처리): 컨텍스트=%s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
		return false;
	}

	return PlayableResolve->ConsumeMana(InMana);
}

void ACMagicSkillContext::Destroy_FromMagicBlueprint()
{
	Destroy();
}

void ACMagicSkillContext::Magic_PlayActionRunExecutionBranches(
	const FDoActionData& InRow,
	ACPlayableCharacter* OptionalPlayableActor)
{
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
			TEXT("[MagicSkillContext] PlayAction: Moving 무효 — Move/카메라 분기 생략 — %s"),
			*GetNameSafe(this)));

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		const FVector SfxLocation =
			IsValid(Character) ? Character->GetActorLocation() : FVector::ZeroVector;

		const TWeakObjectPtr<UWorld> WorldWeak(World);
		if (IsValid(InRow.Effect))
			InRow.PlayEffect(WorldWeak, SfxLocation);

		if (IsValid(InRow.Sound))
			UGameplayStatics::PlaySoundAtLocation(World, InRow.Sound, SfxLocation);
	}

	if (true == InRow.bUseMotionTrail && IsValid(OptionalPlayableActor))
		OptionalPlayableActor->SetVisibleMotionTrail();

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
		const FVector LaunchVelocity(
			RightScaled.X,
			RightScaled.Y,
			UpScaled.Z);

		CharacterLauncher->LaunchCharacter(LaunchVelocity, false, false);
	}
}

ACMagicAroundSkillContext::ACMagicAroundSkillContext()
{
}

void ACMagicAroundSkillContext::DoAction_Implementation(CEAttackType InAttackType, int32 InSkillIndex)
{
	(void)InSkillIndex;

	ACMagicSkillContext::DoAction_Implementation(InAttackType, InSkillIndex);

	UCStateComponent* StateComp = Magic_ResolveStateComponent();
	const int32 RowCount = DoActionDatas.Num();
	if (RowCount <= 0)
	{
		return;
	}

	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicAroundSkillContext] DoAction: DoAction 행은 있으나 UCStateComponent 를 찾지 못함 — %s Rows=%d"),
			*GetNameSafe(this),
			RowCount));
		return;
	}

	// 레거시: Idle 또는 Riding 일 때만 SetAction 후 PlayAction(0).
	const bool bIdleOrRiding = StateComp->IsIdle() || StateComp->IsRiding();
	if (false == bIdleOrRiding)
	{
		return;
	}

	StateComp->SetAction();
	PlayAction(DoActionDatas, 0);
}

void ACMagicAroundSkillContext::Begin_DoAction_Implementation(CEAttackType InAttackType)
{
	ACMagicSkillContext::Begin_DoAction_Implementation(InAttackType);

	Around_SpawnRandomSkillActorFromSkillClasses();
}

void ACMagicAroundSkillContext::Around_SpawnRandomSkillActorFromSkillClasses()
{
	const int32 ClassCount = SkillClasses.Num();
	if (ClassCount <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicAroundSkillContext] SkillClasses 비어 있음 — 스폰 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	UWorld* World = GetWorld();
	if (false == IsValid(World))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicAroundSkillContext] World 무효 — 스폰 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	const int32 PickedIndex = FMath::RandRange(0, ClassCount - 1);
	TSubclassOf<AActor> PickClass = SkillClasses[PickedIndex];
	if (nullptr == PickClass)
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicAroundSkillContext] SkillClasses[%d] 무효 — 스폰 생략 — %s"),
			PickedIndex,
			*GetNameSafe(this)));
		return;
	}

	// 레거시 BP: SpawnTransform.Identity + Owner=this — 실행 위치가 기본이었으므로 동일 이름으로 둔다(필요 시 BP 가 상대좌표로 보정).
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform SpawnTransform = FTransform::Identity;
	AActor* SpawnedSkill = World->SpawnActor<AActor>(PickClass, SpawnTransform, SpawnParams);
	if (false == IsValid(SpawnedSkill))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicAroundSkillContext] SpawnActor 실패 — Class=%s — %s"),
			*GetNameSafe(PickClass.Get()),
			*GetNameSafe(this)));
	}
}

ACWeaponSkillContext::ACWeaponSkillContext()
{
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	PrimaryActorTick.bCanEverTick = true;
}

void ACWeaponSkillContext::BeginPlay()
{
	Super::BeginPlay();

	// 레거시 ExecutionSequence 순서(Set Character→Set Component→Load Equip→Load DoAction→Load Hit).
	Weapon_SetCharacterAndComponentsFromBlueprint();
	Weapon_ClearRuntimeTableCaches();
	Weapon_LoadEquipDatasFromTable();
	Weapon_LoadDoActionDatasFromTables();
	Weapon_LoadHitDatasFromTable();
}

void ACWeaponSkillContext::Tick(float DeltaSeconds)
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

void ACWeaponSkillContext::PlayAction(
	const TArray<FDoActionData>& InDatas,
	const int32 InIndex,
	const FVector& InHitPoint)
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] PlayAction(Character 없음): %s Owner=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetOwner())));
		return;
	}

	if (false == InDatas.IsValidIndex(InIndex))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] PlayAction(인덱스 범위 밖 — %s): Index=%d Count=%d"),
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

void ACWeaponSkillContext::End_DoAction(CEAttackType InAttackType)
{
	// 레거시 Weapon_End_DoAction 그래프: InAction 해제 → Move/UnFixCamera → RestoreState 이후 활성 UCAct 종료.
	InAction = false;

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] End_DoAction(Character 없음 — 이동·복구 생략): %s attack=%d"),
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
			TEXT("[WeaponSkillContext] End_DoAction: UCWeaponComponent 없음 — Character=%s"),
			*GetNameSafe(Character)));
	}
}

void ACWeaponSkillContext::RestorePrevState()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] RestorePrevState(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	Character->ApplyRestoreStateFromPrevMode();
}

bool ACWeaponSkillContext::ConsumeStamina(const double InStamina)
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] ConsumeStamina(Character 없음): %s"),
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

void ACWeaponSkillContext::Destroy_FromWeaponBlueprint()
{
	Destroy();
}

void ACWeaponSkillContext::DoAction(const CEAttackType InAttackType, const int32 InSkillIndex)
{
	Weapon_DoActionImpl(InAttackType, InSkillIndex);
}

void ACWeaponSkillContext::Weapon_DoActionImpl(const CEAttackType InAttackType, const int32 InSkillIndex)
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

void ACWeaponSkillContext::Begin_DoAction_Implementation(const CEAttackType InAttackType)
{
	(void)InAttackType;
}

UCStateComponent* ACWeaponSkillContext::Weapon_ResolveStateComponent() const
{
	if (false == IsValid(Character))
		return nullptr;

	UCStateComponent* StateComp = Cast<UCStateComponent>(State);
	if (IsValid(StateComp))
		return StateComp;

	return Character->FindComponentByClass<UCStateComponent>();
}

bool ACWeaponSkillContext::Weapon_ShouldAirDashFromMovementState() const
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

void ACWeaponSkillContext::Pressed_Implementation()
{
	CancelDashes();
}

void ACWeaponSkillContext::Released_Implementation()
{
}

void ACWeaponSkillContext::Hold_SubWeapon_Implementation()
{
}

void ACWeaponSkillContext::Released_SubWeapon_Implementation()
{
}

void ACWeaponSkillContext::DoSubWeaponAction_Implementation()
{
	CancelDashes();
}

void ACWeaponSkillContext::Begin_DoSubWeaponAction_Implementation()
{
	InSubWeaponAction = true;
}

void ACWeaponSkillContext::End_DoSubWeaponAction_Implementation()
{
	InSubWeaponAction = false;
}

void ACWeaponSkillContext::Dash()
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
			TEXT("[WeaponSkillContext] Dash: UCStateComponent 없어 SetDash 생략 — %s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
	}
}

void ACWeaponSkillContext::GroundDash()
{
	InGroundDashing = true;
	InAirDashing = false;
	PlayAction(DoMovingActionDatas, GroundDashIndex, FVector::ZeroVector);
}

void ACWeaponSkillContext::End_GroundDash()
{
	InGroundDashing = false;
	InAirDashing = false;
	RestorePrevState();
}

void ACWeaponSkillContext::AirDash()
{
	UCMovementComponent* MoveRef = Moving;
	if (false == IsValid(MoveRef) && IsValid(Character))
	{
		MoveRef = Character->FindComponentByClass<UCMovementComponent>();
	}

	if (false == IsValid(MoveRef))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] AirDash: Moving/UCMovementComponent 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	MoveRef->SetLerpMove(true);
	InAirDashing = true;
	InGroundDashing = false;
	PlayAction(DoMovingActionDatas, AirDashIndex, FVector::ZeroVector);
}

void ACWeaponSkillContext::End_AirDash()
{
	InAirDashing = false;
	InGroundDashing = false;

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] End_AirDash(Character 없음): %s"),
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

void ACWeaponSkillContext::CancelDashes()
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

void ACWeaponSkillContext::SetControllerInEquip()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] SetControllerInEquip(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	Controller = Character->CurController.Get();
}

void ACWeaponSkillContext::Equip_Implementation()
{
	CancelDashes();

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] Equip(Character 없음): %s"),
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
			TEXT("[WeaponSkillContext] Equip: UCStateComponent 없음 — Character=%s 무기=%s"),
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

void ACWeaponSkillContext::Unequip_Implementation()
{
	CancelDashes();

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] Unequip(Character 없음): %s"),
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
			TEXT("[WeaponSkillContext] Unequip: Moving 미연결 — %s"),
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

void ACWeaponSkillContext::Begin_Equip_Implementation(bool bMainOrSubWeapon)
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] Begin_Equip(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	if (bMainOrSubWeapon)
		IsEquipping = true;

	if (IsValid(EquipData.Sound))
		EquipData.PlaySoundWave(TWeakObjectPtr<ACCommonCharacter>(Character));
}

void ACWeaponSkillContext::End_Equip_Implementation(bool bMainOrSubWeapon)
{
	if (false == bMainOrSubWeapon)
		return;

	IsEquipping = false;
	InEquip = true;

	if (IsValid(Moving))
		Moving->Move();

	RestorePrevState();
}

void ACWeaponSkillContext::Begin_Unequip_Implementation(bool bMainOrSubWeapon)
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] Begin_Unequip(Character 없음): %s"),
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

void ACWeaponSkillContext::End_Unequip_Implementation(bool bMainOrSubWeapon)
{
	if (false == bMainOrSubWeapon)
		return;

	IsUnequipping = false;
	InEquip = false;

	if (IsValid(Moving))
		Moving->Move();
}

void ACWeaponSkillContext::Weapon_ApplyEquipRowFacingAndMovement(const FEquipData& Row)
{
	if (false == IsValid(Moving))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] Equip 이동 분기 중 Moving 미연결 — %s"),
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

void ACWeaponSkillContext::Weapon_PlayActionRunExecutionBranches(
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
			TEXT("[WeaponSkillContext] PlayAction: Moving 무효 — Move/카메라 분기 생략 — %s"),
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

void ACWeaponSkillContext::Weapon_SetCharacterAndComponentsFromBlueprint()
{
	Character = Cast<ACCommonCharacter>(GetOwner());

	if (false == IsValid(Character))
	{
		Moving = nullptr;
		State = nullptr;
		// 무기 칠드액터에 Owner 미달 시 이후 표 로드만 시도된다 — 디버깅 시 추적할 수 있게 한 번 로그한다.
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] BeginPlay(Set Character 실패): Owner 가 ACCommonCharacter 가 아니거나 무효입니다. Actor=%s Owner=%s"),
			*GetName(),
			IsValid(GetOwner()) ? *GetOwner()->GetName() : TEXT("(null)")));
		return;
	}

	Moving = Character->FindComponentByClass<UCMovementComponent>();
	if (false == IsValid(Moving))
		CLog::Log(FString::Printf(
			TEXT("[WeaponSkillContext] 캐릭터 %s 에 UCMovementComponent 없음 — Moving 미연결."), *Character->GetName()));

	State = Character->FindComponentByClass<UCStateComponent>();
	// 상태 컴포넌트 미부착 캐릭터 클래스도 있을 수 있어 여기선 침묵한다.
}

void ACWeaponSkillContext::Weapon_ClearRuntimeTableCaches()
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

void ACWeaponSkillContext::Weapon_LoadEquipDatasFromTable()
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
				TEXT("[WeaponSkillContext] EquipDataTable '%s' 첫 행 '%s' 를 FEquipData 로 읽지 못했습니다."),
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
				TEXT("[WeaponSkillContext] EquipDataTable '%s' 둘째 행 '%s' 를 FEquipData 로 읽지 못했습니다."),
				*EquipDataTable->GetName(),
				*RowNames[1].ToString()));
	}
}

void ACWeaponSkillContext::Weapon_AppendDoActionRowByAttackType(
	ACWeaponSkillContext* InWeapon,
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

void ACWeaponSkillContext::Weapon_AppendMovingDoActionRowIfDashTypes(
	ACWeaponSkillContext* InWeapon,
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

void ACWeaponSkillContext::Weapon_LoadDoActionDatasFromTables()
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

void ACWeaponSkillContext::Weapon_AppendHitRowByAttackType(
	ACWeaponSkillContext* InWeapon,
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

void ACWeaponSkillContext::Weapon_LoadHitDatasFromTable()
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

ACWeaponComboSkillContext::ACWeaponComboSkillContext()
{
}

ACWeaponGuardSkillContext::ACWeaponGuardSkillContext()
{
}

namespace WeaponGuard_Local
{
	static UShapeComponent* FindShape(ACWeaponGuardSkillContext* Self, const int32 IndexRaw)
	{
		if (false == IsValid(Self))
		{
			return nullptr;
		}

		if (Self->Collisions.Num() <= 0)
		{
			return nullptr;
		}

		const int32 Clamped = Self->Collisions.IsValidIndex(IndexRaw)
			? IndexRaw
			: FMath::Clamp(IndexRaw, 0, Self->Collisions.Num() - 1);
		UShapeComponent* Shape = Self->Collisions[Clamped].Get();
		if (false == IsValid(Shape))
		{
			ensureMsgf(false, TEXT("[WeaponGuardSkillContext] Collisions[%d] Shape 무효 — %s"),
				Clamped,
				*GetNameSafe(Self));
		}
		return Shape;
	}

	static UPrimitiveComponent* FindAnyWeaponOverlapPrim(ACWeaponSkillContext* WeaponCtx)
	{
		if (false == IsValid(WeaponCtx))
		{
			return nullptr;
		}

		TArray<UShapeComponent*> Scratch;
		WeaponCtx->GetComponents<UShapeComponent>(Scratch, true);
		if (Scratch.Num() <= 0)
		{
			return nullptr;
		}

		return Scratch[0];
	}
}

void ACWeaponGuardSkillContext::BeginPlay()
{
	Super::BeginPlay();

	Collisions.Reset();
	TArray<UShapeComponent*> ScratchShapes;
	GetComponents<UShapeComponent>(ScratchShapes, true);
	Collisions.Reserve(ScratchShapes.Num());
	for (UShapeComponent* ShapeCandidate : ScratchShapes)
	{
		if (false == IsValid(ShapeCandidate))
			continue;

		Collisions.Add(ShapeCandidate);
	}

	OffGuardCollsion();
	OffSubWeaponCollision();
	Guard_BindOverlapsRecursive();
}

void ACWeaponGuardSkillContext::Guard_BindOverlapsRecursive()
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : Collisions)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		UPrimitiveComponent* Prim = ShapePtr;
		Prim->OnComponentBeginOverlap.AddDynamic(this, &ACWeaponGuardSkillContext::Guard_OnShapeBeginOverlap_Dynamic);
	}
}

void ACWeaponGuardSkillContext::Guard_UnbindOverlapsRecursive()
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : Collisions)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		UPrimitiveComponent* Prim = ShapePtr;
		Prim->OnComponentBeginOverlap.RemoveDynamic(this,
			&ACWeaponGuardSkillContext::Guard_OnShapeBeginOverlap_Dynamic);
	}
}

void ACWeaponGuardSkillContext::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Guard_UnbindOverlapsRecursive();
	Collisions.Reset();

	UWorld* WorldForTimers = GetWorld();
	if (IsValid(WorldForTimers))
	{
		FTimerManager& TM = WorldForTimers->GetTimerManager();
		TM.ClearTimer(GuardParriableResetTimerHandle);
		TM.ClearTimer(GuardParriedReflectTimerHandle);
		TM.ClearTimer(GuardClearHittedTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void ACWeaponGuardSkillContext::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UWorld* World = GetWorld();
	if (false == IsValid(World))
	{
		return;
	}

	FTimerManager& TM = World->GetTimerManager();

	if (Parriable)
	{
		if (false == TM.IsTimerActive(GuardParriableResetTimerHandle))
		{
			TM.SetTimer(GuardParriableResetTimerHandle,
				FTimerDelegate::CreateUObject(this, &ACWeaponGuardSkillContext::Guard_Timer_ClearParriableElapsed),
				static_cast<float>(ParriableDelay),
				false);
		}
	}

	if (Parried)
	{
		Parried = false;
		if (false == TM.IsTimerActive(GuardParriedReflectTimerHandle))
		{
			TM.SetTimer(GuardParriedReflectTimerHandle,
				FTimerDelegate::CreateUObject(this, &ACWeaponGuardSkillContext::Guard_Timer_ParriedReflectElapsed),
				static_cast<float>(ParriedDelay),
				false);
		}
	}

	if (ClearHitted)
	{
		if (false == TM.IsTimerActive(GuardClearHittedTimerHandle))
		{
			TM.SetTimer(GuardClearHittedTimerHandle,
				FTimerDelegate::CreateUObject(this, &ACWeaponGuardSkillContext::Guard_Timer_ClearHittedListElapsed),
				GuardClearHittedDelaySeconds,
				false);
		}
	}
}

void ACWeaponGuardSkillContext::Guard_Timer_ClearParriableElapsed()
{
	Parriable = false;
}

void ACWeaponGuardSkillContext::Guard_Timer_ParriedReflectElapsed()
{
	ACharacter* TargetRaw = Target.Get();
	ACCommonCharacter* TargetChar = Cast<ACCommonCharacter>(TargetRaw);

	if (!ensureMsgf(IsValid(TargetChar), TEXT("[WeaponGuardSkillContext] 패링 반사 타겟은 ACCommonCharacter 여야 함 — %s"),
			*GetNameSafe(TargetRaw)))
	{
		Target = nullptr;
		return;
	}

	if (!ensureMsgf(IsValid(Character), TEXT("[WeaponGuardSkillContext] 패링 반사: 소유 Character 없음 — %s"),
			*GetNameSafe(this)))
	{
		Target = nullptr;
		return;
	}

	if (!ensureMsgf(HitCommonDatas.IsValidIndex(0),
			TEXT("[WeaponGuardSkillContext] 패링 반사: HitCommonDatas[0] 필요 — %s"),
			*GetNameSafe(this)))
	{
		Target = nullptr;
		return;
	}

	HitCommonDatas[0].SendDamage(
		TWeakObjectPtr<ACCommonCharacter>(Character),
		TWeakObjectPtr<AActor>(this),
		TWeakObjectPtr<ACCommonCharacter>(TargetChar));

	Target = nullptr;
}

void ACWeaponGuardSkillContext::Guard_Timer_ClearHittedListElapsed()
{
	ClearHitted = false;
	Hitted.Reset();
}

void ACWeaponGuardSkillContext::OnGuardCollision()
{
	UShapeComponent* Shape = WeaponGuard_Local::FindShape(this, GuardCollisionIndex);
	if (!ensureMsgf(IsValid(Shape),
			TEXT("[WeaponGuardSkillContext] OnGuardCollision: GuardCollisionIndex Shape 없음(idx=%d) — %s"),
			GuardCollisionIndex,
			*GetNameSafe(this)))
	{
		return;
	}

	Shape->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ACWeaponGuardSkillContext::OffGuardCollsion()
{
	UShapeComponent* Shape = WeaponGuard_Local::FindShape(this, GuardCollisionIndex);
	if (!ensureMsgf(IsValid(Shape),
			TEXT("[WeaponGuardSkillContext] OffGuardCollsion: Shape 없음(idx=%d) — %s"),
			GuardCollisionIndex,
			*GetNameSafe(this)))
	{
		return;
	}

	Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACWeaponGuardSkillContext::OnSubWeaponCollision()
{
	UShapeComponent* Shape = WeaponGuard_Local::FindShape(this, ActionCollisionIndex);
	if (!ensureMsgf(IsValid(Shape),
			TEXT("[WeaponGuardSkillContext] OnSubWeaponCollision: Shape 없음(idx=%d) — %s"),
			ActionCollisionIndex,
			*GetNameSafe(this)))
	{
		return;
	}

	Shape->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ACWeaponGuardSkillContext::OffSubWeaponCollision()
{
	UShapeComponent* Shape = WeaponGuard_Local::FindShape(this, ActionCollisionIndex);
	if (!ensureMsgf(IsValid(Shape),
			TEXT("[WeaponGuardSkillContext] OffSubWeaponCollision: Shape 없음(idx=%d) — %s"),
			ActionCollisionIndex,
			*GetNameSafe(this)))
	{
		return;
	}

	Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACWeaponGuardSkillContext::Guard_OnShapeBeginOverlap_Dynamic(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	UShapeComponent* OverlapShape = Cast<UShapeComponent>(OverlappedComponent);
	if (!ensureMsgf(IsValid(OverlapShape),
			TEXT("[WeaponGuardSkillContext] 가드 오버랩 바인 대상은 Shape 여야 함 — %s"),
			*GetNameSafe(OverlappedComponent)))
	{
		return;
	}

	if (false == IsValid(Character))
	{
		return;
	}

	ACCommonCharacter* IncomingAttackerChar = nullptr;
	UPrimitiveComponent* TraceOtherPrim = OtherComp;

	ACWeaponSkillContext* OverlapWeaponCtx = Cast<ACWeaponSkillContext>(OtherActor);
	if (IsValid(OverlapWeaponCtx))
	{
		IncomingAttackerChar = OverlapWeaponCtx->Character;
		if (false == IsValid(IncomingAttackerChar))
		{
			IncomingAttackerChar = Cast<ACCommonCharacter>(OverlapWeaponCtx->GetOwner());
		}

		UPrimitiveComponent* WeaponShapeHint = WeaponGuard_Local::FindAnyWeaponOverlapPrim(OverlapWeaponCtx);
		if (IsValid(WeaponShapeHint))
		{
			TraceOtherPrim = WeaponShapeHint;
		}
	}
	else
	{
		IncomingAttackerChar = Cast<ACCommonCharacter>(OtherActor);
	}

	if (false == IsValid(IncomingAttackerChar))
	{
		return;
	}

	if (UCYJJBlueprintLibrary::AreCharactersSameGroup(Character, IncomingAttackerChar))
	{
		return;
	}

	FVector ImpactWorld =
		IsValid(TraceOtherPrim) ? TraceOtherPrim->K2_GetComponentLocation() : IncomingAttackerChar->GetActorLocation();

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Character);

	bool TraceOk = UCYJJBlueprintLibrary::TryGetHitPointBetweenPrimitives(
		OverlapShape,
		TraceOtherPrim,
		GuardHitSphereRadius,
		IgnoreActors,
		ImpactWorld);

	if (false == TraceOk)
	{
		if (true == IsValid(TraceOtherPrim))
		{
			ImpactWorld = TraceOtherPrim->K2_GetComponentLocation();
		}
		else
		{
			ImpactWorld = IncomingAttackerChar->GetActorLocation();
		}
	}

	OnBeginOverlap(IncomingAttackerChar, ImpactWorld);
}

void ACWeaponGuardSkillContext::OnBeginOverlap_Implementation(ACCommonCharacter* InCharacter,
	const FVector& InHitPoint)
{
	if (!ensureMsgf(IsValid(InCharacter), TEXT("[WeaponGuardSkillContext] OnBeginOverlap: InCharacter 필요")))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!ensureMsgf(IsValid(World), TEXT("[WeaponGuardSkillContext] OnBeginOverlap: World 필요 — %s"), *GetNameSafe(this)))
	{
		return;
	}

	FTimerManager* TimerMgr = &World->GetTimerManager();

	if (Guarding)
	{
		TArray<ACCommonCharacter*> Scratch;
		Scratch.Reserve(Hitted.Num() + 2);
		for (TObjectPtr<ACCommonCharacter>& EntryWeak : Hitted)
		{
			ACCommonCharacter* EntryRaw = EntryWeak.Get();
			if (IsValid(EntryRaw))
			{
				Scratch.Add(EntryRaw);
			}
		}

		const bool bInserted = UCYJJBlueprintLibrary::TryAddUniqueCommonCharacter(Scratch, InCharacter);

		Hitted.Reset();
		Hitted.Reserve(Scratch.Num());
		for (ACCommonCharacter* Added : Scratch)
		{
			Hitted.Add(Added);
		}

		if (bInserted)
		{
			const FVector SoundLocation =
				IsValid(Character) ? Character->GetActorLocation() : GetActorLocation();

			if (IsValid(GuardBlockSoundAsset))
			{
				UGameplayStatics::PlaySoundAtLocation(World,
					GuardBlockSoundAsset,
					SoundLocation,
					FRotator::ZeroRotator,
					1.0f,
					1.0f,
					0.0f,
					GuardBlockAttenuation);
			}

			ClearHitted = true;
		}
	}

	if (Parriable)
	{
		if (!ensureMsgf(DoActionDatas.IsValidIndex(ParryingAnimIndex),
				TEXT("[WeaponGuardSkillContext] 패링 재생 실패: DoActionDatas 에 인덱스 %d 없음(행 수 %d) — %s"),
				ParryingAnimIndex,
				DoActionDatas.Num(),
				*GetNameSafe(this)))
		{
			return;
		}

		TimerMgr->ClearTimer(GuardParriableResetTimerHandle);

		Parriable = false;
		Guarding = false;
		Parrying = true;

		PlayAction(DoActionDatas, ParryingAnimIndex, InHitPoint);

		Parried = true;
		Target = InCharacter;
	}
}

void ACWeaponGuardSkillContext::Hold_SubWeapon_Implementation()
{
	Parriable = true;

	if (Parrying)
	{
		// 레거시 그래프: Parrying 분기의 then 은 빈 결과 — Guarding/PlayAction 을 진행하지 않는다(Idle 분기 진입 불가와 동등).
		return;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (!ensureMsgf(IsValid(StateComp), TEXT("[WeaponGuardSkillContext] Hold_SubWeapon: StateComponent 필요 — %s"),
			*GetNameSafe(this)))
	{
		return;
	}

	if (false == StateComp->IsIdle() && false == StateComp->IsHitted())
	{
		return;
	}

	if (!ensureMsgf(DoActionDatas.Num() > 0, TEXT("[WeaponGuardSkillContext] Hold_SubWeapon: DoActionDatas 필요 — %s"),
			*GetNameSafe(this)))
	{
		return;
	}

	Guarding = true;

	const int32 ClampedGuardIndex = FMath::Clamp(GuardAnimIndex, 0, DoActionDatas.Num() - 1);
	const FVector HitPointZero = FVector::ZeroVector;
	PlayAction(DoActionDatas, ClampedGuardIndex, HitPointZero);
}

void ACWeaponGuardSkillContext::Released_SubWeapon_Implementation()
{
	Guarding = false;

	ACharacter* CharActor = IsValid(Character) ? Cast<ACharacter>(Character) : nullptr;
	if (!ensureMsgf(IsValid(CharActor), TEXT("[WeaponGuardSkillContext] Released_SubWeapon: ACharacter 필요 — %s"),
			*GetNameSafe(this)))
	{
		return;
	}

	if (!ensureMsgf(IsValid(GuardHoldStopMontage),
			TEXT("[WeaponGuardSkillContext] Released_SubWeapon: GuardHoldStopMontage 할당 필요 — %s"),
			*GetNameSafe(this)))
	{
		return;
	}

	CharActor->StopAnimMontage(GuardHoldStopMontage);

	if (!ensureMsgf(IsValid(Moving), TEXT("[WeaponGuardSkillContext] Released_SubWeapon: Moving 필요 — %s"),
			*GetNameSafe(this)))
	{
		return;
	}

	Moving->Move();
}

void ACWeaponGuardSkillContext::DoSubWeaponAction_Implementation()
{
	// 레거시 `Weapon_Guard:DoSubWeaponAction` — 노드 미연결로 본문 없음(`Weapon` 기본의 `CancelDashes` 도 호출 안 함).
}

void ACWeaponGuardSkillContext::Begin_DoSubWeaponAction_Implementation()
{
	Super::Begin_DoSubWeaponAction_Implementation();

	UWorld* World = GetWorld();
	if (!ensureMsgf(IsValid(World), TEXT("[WeaponGuardSkillContext] Begin_DoSubWeaponAction: World 필요 — %s"),
			*GetNameSafe(this)))
	{
		return;
	}

	if (!ensureMsgf(HitCommonDatas.Num() > 0,
			TEXT("[WeaponGuardSkillContext] Begin_DoSubWeaponAction: HitCommonDatas 필요 — %s"),
			*GetNameSafe(this)))
	{
		return;
	}

	const float HitStopValue = HitCommonDatas[0].HitStop;

	TArray<AActor*> Candidates;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Candidates);

	for (AActor* Candidate : Candidates)
	{
		if (false == IsValid(Candidate))
		{
			continue;
		}

		bool bShouldDilate = false;
		const APawn* AsPawn = Cast<APawn>(Candidate);
		if (IsValid(AsPawn))
		{
			bShouldDilate = true;
		}
		else
		{
			UMeshComponent* MeshComp = Candidate->FindComponentByClass<UMeshComponent>();
			if (IsValid(MeshComp) && MeshComp->Mobility == EComponentMobility::Movable)
			{
				bShouldDilate = true;
			}
		}

		if (false == bShouldDilate)
		{
			continue;
		}

		DilationActors.Add(Candidate);
		Candidate->CustomTimeDilation = HitStopValue;
	}
}

void ACWeaponGuardSkillContext::End_DoSubWeaponAction_Implementation()
{
	Parrying = false;

	for (TObjectPtr<AActor>& EntryWeak : DilationActors)
	{
		AActor* ActorPtr = EntryWeak.Get();
		if (IsValid(ActorPtr))
		{
			ActorPtr->CustomTimeDilation = 1.0f;
		}
	}

	DilationActors.Reset();

	Super::End_DoSubWeaponAction_Implementation();
}

void ACWeaponComboSkillContext::BeginPlay()
{
	Super::BeginPlay();

	ComboCollisionShapes.Reset();
	GetComponents<UShapeComponent>(ComboCollisionShapes, true);
	OffCollisions();

	for (TObjectPtr<UShapeComponent>& ShapeWeak : ComboCollisionShapes)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		UPrimitiveComponent* PrimPtr = ShapePtr;

		PrimPtr->OnComponentBeginOverlap.AddDynamic(
			this,
			&ACWeaponComboSkillContext::ComboOnShapeBeginOverlap_Impl);

		PrimPtr->OnComponentHit.AddDynamic(this, &ACWeaponComboSkillContext::ComboOnShapeHit_Impl);
	}
}

void ACWeaponComboSkillContext::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : ComboCollisionShapes)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		UPrimitiveComponent* PrimPtr = ShapePtr;
		PrimPtr->OnComponentBeginOverlap.RemoveDynamic(
			this,
			&ACWeaponComboSkillContext::ComboOnShapeBeginOverlap_Impl);
		PrimPtr->OnComponentHit.RemoveDynamic(this, &ACWeaponComboSkillContext::ComboOnShapeHit_Impl);
	}

	ComboCollisionShapes.Reset();

	UWorld* WorldForTimer = GetWorld();
	if (IsValid(WorldForTimer))
	{
		WorldForTimer->GetTimerManager().ClearTimer(ComboFallDownGravityTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void ACWeaponComboSkillContext::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Combo_Tick_TargetingDelegation();
}

void ACWeaponComboSkillContext::OnCollisions()
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : ComboCollisionShapes)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		ShapePtr->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void ACWeaponComboSkillContext::OffCollisions()
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : ComboCollisionShapes)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		ShapePtr->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 레거시 Weapon_Combo::OffCollisions: Shape 끄고 → Moving 고정 카메라면 Hitted 를 후보로 전방 1명 골라 RotationTarget/Targeting/Mouse 채운 뒤 → Hitted 클리어.
	const bool bRunFixedCameraTargeting =
		IsValid(Character) && IsValid(Moving) && Moving->GetFixedCamera();

	if (bRunFixedCameraTargeting)
	{
		TArray<ACharacter*> HitCandidates;
		HitCandidates.Reserve(ComboHitted.Num());

		for (TObjectPtr<ACCommonCharacter>& HitWeak : ComboHitted)
		{
			ACCommonCharacter* HitCharacter = HitWeak.Get();
			if (IsValid(HitCharacter))
			{
				HitCandidates.Add(HitCharacter);
			}
		}

		ACharacter* FrontCandidate = UCYJJBlueprintLibrary::FindNearlyFrontCharacter(
			Character,
			HitCandidates,
			0.7);

		if (IsValid(FrontCandidate))
		{
			const FVector CharacterLocation = Character->GetActorLocation();
			const FVector FrontLocation = FrontCandidate->GetActorLocation();
			ComboRotationTarget = UKismetMathLibrary::FindLookAtRotation(CharacterLocation, FrontLocation);
			ComboTargetingFlag = true;
			ComboMousePosition = UWidgetLayoutLibrary::GetMousePositionOnPlatform();
		}
	}

	ComboHitted.Reset();
}

void ACWeaponComboSkillContext::EnableCombo()
{
	ComboEnable = true;
}

void ACWeaponComboSkillContext::DisableCombo()
{
	ComboEnable = false;
}

void ACWeaponComboSkillContext::Combo_Tick_FixedCameraLookTarget_MouseEnd()
{
	// Tick 경로: 매 프레임 호출로 자주 실패 → 로그 없음(프로젝트 규칙).
	if (false == ComboTargetingFlag || false == IsValid(Character))
	{
		return;
	}

	AController* ControllerResolved = ComboResolveControllerForWeaponContext(this);

	if (IsValid(ControllerResolved))
	{
		const float DeltaSeconds = static_cast<float>(UGameplayStatics::GetWorldDeltaSeconds(GetWorld()));

		const FRotator CurrentRotation = ControllerResolved->GetControlRotation();
		const FRotator InterpRotation = UKismetMathLibrary::RInterpTo(
			CurrentRotation,
			ComboRotationTarget,
			DeltaSeconds,
			ComboTickTargetingInterpSpeed);

		ControllerResolved->SetControlRotation(InterpRotation);
	}

	const FVector2D MouseNow = UWidgetLayoutLibrary::GetMousePositionOnPlatform();
	const bool bMouseMoved = false
		== UKismetMathLibrary::EqualEqual_Vector2DVector2D(
			MouseNow,
			ComboMousePosition,
			ComboTickTargetingMouseTolerance);

	FRotator RotationAfter = FRotator::ZeroRotator;
	if (IsValid(ControllerResolved))
	{
		RotationAfter = ControllerResolved->GetControlRotation();
	}
	else if (IsValid(Character))
	{
		RotationAfter = Character->GetControlRotation();
	}

	const bool bReachedLookTarget = UKismetMathLibrary::EqualEqual_RotatorRotator(
		RotationAfter,
		ComboRotationTarget,
		ComboTickTargetingRotatorTolerance);

	if (bMouseMoved || bReachedLookTarget)
	{
		ComboTargetingFlag = false;
		if (IsValid(Moving))
		{
			Moving->UnFixCamera();
		}
	}
}

void ACWeaponComboSkillContext::Combo_Tick_TargetingDelegation()
{
	if (false == InEquip || false == IsValid(Character))
	{
		return;
	}

	// 고정 카메라 타격 후 회전 보간 — `UCTargetingComponent` 잠금과 별도 변수(`ComboTargetingFlag`)로 동작.
	if (true == ComboTargetingFlag)
	{
		Combo_Tick_FixedCameraLookTarget_MouseEnd();
		return;
	}

	UCTargetingComponent* TargetingCompRaw = Character->GetTargetingComp().Get();
	if (false == IsValid(TargetingCompRaw))
	{
		return;
	}

	// 컴포넌트 틱이 켜져 있으면 해당 틱 안에서 이미 Tick_Targeting 이 돌아간다.
	if (TargetingCompRaw->IsComponentTickEnabled())
	{
		return;
	}

	TargetingCompRaw->Tick_Targeting();
}

bool ACWeaponComboSkillContext::Combo_ResolveHitDatasForMelee(
	ACWeaponSkillContext* Weapon,
	const CEAttackType AttackTypeBranch,
	const int32 IndexCommon,
	const int32 IndexAir,
	const int32 IndexFlying,
	const int32 IndexDown,
	const int32 IndexSwaying,
	const int32 IndexFallDown,
	const int32 IndexSkill,
	TArray<FHitData>*& OutHitDatasPtr,
	int32& OutClampedHitIndex)
{
	if (false == IsValid(Weapon))
	{
		return false;
	}

	TArray<FHitData>* HitRowArray = nullptr;
	int32 HitIndexDesired = 0;

	switch (AttackTypeBranch)
	{
	case CEAttackType::Common:
		HitRowArray = &Weapon->HitCommonDatas;
		HitIndexDesired = IndexCommon;
		break;
	case CEAttackType::Air:
		HitRowArray = &Weapon->HitAirComboDatas;
		HitIndexDesired = IndexAir;
		break;
	case CEAttackType::Flying:
		HitRowArray = &Weapon->HitFlyingAttackDatas;
		HitIndexDesired = IndexFlying;
		break;
	case CEAttackType::Down:
		HitRowArray = &Weapon->HitDownAttackDatas;
		HitIndexDesired = IndexDown;
		break;
	case CEAttackType::DashAttack:
		HitRowArray = &Weapon->HitSwayingAttackDatas;
		HitIndexDesired = IndexSwaying;
		break;
	case CEAttackType::FallDown:
		HitRowArray = &Weapon->HitFallDownAttackDatas;
		HitIndexDesired = IndexFallDown;
		break;
	case CEAttackType::Skill:
		HitRowArray = &Weapon->HitSkillDatas;
		HitIndexDesired = IndexSkill;
		break;
	default:
		return false;
	}

	if (nullptr == HitRowArray || HitRowArray->Num() <= 0)
	{
		return false;
	}

	OutHitDatasPtr = HitRowArray;
	const int32 MaxIndexInclusive = HitRowArray->Num() - 1;
	OutClampedHitIndex = FMath::Clamp(HitIndexDesired, 0, MaxIndexInclusive);

	return true;
}

bool ACWeaponComboSkillContext::Combo_AttemptApplyHitAgainst(
	UPrimitiveComponent* WeaponPrimitive,
	UPrimitiveComponent* OtherPrim,
	ACCommonCharacter* HitCharacter,
	const FVector& InHitWorldLocation)
{
	(void)WeaponPrimitive;

	if (false == IsValid(Character) || false == IsValid(HitCharacter))
	{
		return false;
	}

	UCCharacterInfoComponent* OwnerInfoRaw = Character->FindComponentByClass<UCCharacterInfoComponent>();
	if (IsValid(OwnerInfoRaw) && OwnerInfoRaw->IsSameGroup(HitCharacter))
	{
		return false;
	}

	for (TObjectPtr<ACCommonCharacter>& ExistingWeak : ComboHitted)
	{
		if (ExistingWeak.Get() == HitCharacter)
		{
			return false;
		}
	}

	TArray<FHitData>* HitRows = nullptr;
	int32 HitIx = 0;
	if (false ==
		Combo_ResolveHitDatasForMelee(
			this,
			CurAttackType,
			ComboHitIndexCommon,
			ComboHitIndexAir,
			ComboHitIndexFlying,
			ComboHitIndexDown,
			ComboHitIndexSwaying,
			ComboHitIndexFallDown,
			ComboHitIndexSkill,
			HitRows,
			HitIx))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] HitData 없음 또는 미지원 AttackType=%d (%s): 대상=%s"),
			static_cast<int32>(CurAttackType),
			*GetNameSafe(this),
			*GetNameSafe(HitCharacter)));
		return false;
	}

	(void)OtherPrim;

	(*HitRows)[HitIx].SendDamage(TWeakObjectPtr<ACCommonCharacter>(Character), this, HitCharacter);
	ComboHitted.AddUnique(HitCharacter);
	Combo_OnBeginOverlap(HitCharacter, InHitWorldLocation);

	return true;
}

void ACWeaponComboSkillContext::ComboOnShapeBeginOverlap_Impl(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepFromMaybe)
{
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepFromMaybe;

	ACCommonCharacter* HitChar = Cast<ACCommonCharacter>(OtherActor);
	if (false == IsValid(HitChar))
	{
		return;
	}

	if (false == IsValid(Character))
	{
		return;
	}

	TArray<AActor*> ActorsIgnore;
	ActorsIgnore.Reserve(2);
	ActorsIgnore.Add(Character);

	FVector ImpactWorld =
		IsValid(OtherComp) ? OtherComp->K2_GetComponentLocation() : HitChar->GetActorLocation();

	bool TraceOk = UCYJJBlueprintLibrary::TryGetHitPointBetweenPrimitives(
		OverlappedComponent,
		OtherComp,
		ComboMeleeHitSphereRadius,
		ActorsIgnore,
		ImpactWorld);

	if (false == TraceOk)
	{
		if (true == IsValid(OtherComp))
		{
			ImpactWorld = OtherComp->K2_GetComponentLocation();
		}
		else
		{
			ImpactWorld = HitChar->GetActorLocation();
		}
	}

	Combo_AttemptApplyHitAgainst(OverlappedComponent, OtherComp, HitChar, ImpactWorld);
}

void ACWeaponComboSkillContext::ComboOnShapeHit_Impl(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	(void)NormalImpulse;

	ACCommonCharacter* HitChar = Cast<ACCommonCharacter>(OtherActor);
	if (false == IsValid(HitChar))
	{
		return;
	}

	FVector ImpactWorld = FVector::ZeroVector;
	if (true == Hit.bBlockingHit && false == Hit.ImpactPoint.Equals(FVector::ZeroVector))
	{
		ImpactWorld = Hit.ImpactPoint;
	}
	else if (true == IsValid(OtherComp))
	{
		ImpactWorld = OtherComp->K2_GetComponentLocation();
	}
	else
	{
		ImpactWorld = HitChar->GetActorLocation();
	}

	Combo_AttemptApplyHitAgainst(HitComponent, OtherComp, HitChar, ImpactWorld);
}

void ACWeaponComboSkillContext::Combo_OnBeginOverlap_Implementation(
	ACCommonCharacter* InOtherCharacter,
	FVector InHitPoint)
{
	(void)InHitPoint;

	if (false == IsValid(InOtherCharacter))
	{
		return;
	}

	// 레거시 BP Weapon_Combo:OnBeginOverlap — SendDamage(Damaged) 다음 CCProcess 로 히트행의 CrowdControl 을 적용했다.
	// 여기서는 TakeDamage 경로에서 이미 AttackType 기반 피격 타입을 넣은 뒤, CC 가 None 이 아니면 해당 반응으로 덮어쓴다.
	TArray<FHitData>* HitRows = nullptr;
	int32 HitIx = 0;
	if (false ==
		Combo_ResolveHitDatasForMelee(
			this,
			CurAttackType,
			ComboHitIndexCommon,
			ComboHitIndexAir,
			ComboHitIndexFlying,
			ComboHitIndexDown,
			ComboHitIndexSwaying,
			ComboHitIndexFallDown,
			ComboHitIndexSkill,
			HitRows,
			HitIx))
	{
		return;
	}

	const CECrowdControl CrowdControl = (*HitRows)[HitIx].CrowdControl;

	UCStateComponent* VictimState = InOtherCharacter->FindComponentByClass<UCStateComponent>();
	if (false == IsValid(VictimState))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] Combo_OnBeginOverlap: 피격자에 UCStateComponent 없음 — %s"),
			*GetNameSafe(InOtherCharacter)));
		return;
	}

	switch (CrowdControl)
	{
	case CECrowdControl::None:
		break;
	case CECrowdControl::Air:
		VictimState->SetHitReactionAir();
		break;
	case CECrowdControl::PutDown:
		VictimState->SetHitFlyingPutDownMode();
		break;
	case CECrowdControl::Down:
		VictimState->SetHitDownMode();
		break;
	default:
		break;
	}
}

void ACWeaponComboSkillContext::Weapon_DoActionImpl(const CEAttackType InAttackType, const int32 InSkillIndex)
{
	CancelDashes();

	if (true == InAction && InAttackType != PrevAttackType)
	{
		const CEAttackType PrevForEnd = PrevAttackType;
		End_DoAction(PrevForEnd);
		PrevAttackType = InAttackType;
		InAction = true;
		CurAttackType = InAttackType;
		return;
	}

	InAction = true;
	CurAttackType = InAttackType;
	// 레거시 `Weapon` 그래프와 같이 Common/Air/Flying 는 `Begin_Do*` 에서 이동·런치 선행(콤보는 Air 런치·Flying 는 빈 포크).
	Begin_DoAction(InAttackType);
	Combo_DispatchDoAction(InAttackType, InSkillIndex);
}

void ACWeaponComboSkillContext::Begin_DoAction_Implementation(const CEAttackType InAttackType)
{
	CurAttackType = InAttackType;

	switch (InAttackType)
	{
	case CEAttackType::Common:
		ComboFork_Begin_Common();
		break;
	case CEAttackType::Air:
		ComboFork_Begin_Air();
		break;
	case CEAttackType::Flying:
		ComboFork_Begin_Flying();
		break;
	default:
		// 레거시 Begin_DoAction 는 Common/Air/Flying 만 Begin_* 에 연결.
		break;
	}
}

void ACWeaponComboSkillContext::End_DoAction(const CEAttackType InAttackType)
{
	CurAttackType = InAttackType;

	switch (InAttackType)
	{
	case CEAttackType::Common:
		ComboFork_End_Common();
		break;
	case CEAttackType::Air:
		ComboFork_End_Air();
		break;
	case CEAttackType::Flying:
		ComboFork_End_Flying();
		break;
	case CEAttackType::Down:
		ComboFork_End_Down();
		break;
	case CEAttackType::DashAttack:
		ComboFork_End_DashAttack();
		break;
	case CEAttackType::FallDown:
		ComboFork_End_FallDown();
		break;
	case CEAttackType::Skill:
		ComboFork_End_Skill();
		break;
	case CEAttackType::Riding:
		ComboFork_End_Riding();
		break;
	default:
		// 레거시 End_DoAction: Dash / AirDash 등은 종료 분기 미연결(Riding 은 `ComboFork_End_Riding` 처리).
		break;
	}

	ACWeaponSkillContext::End_DoAction(InAttackType);
}

bool ACWeaponComboSkillContext::Combo_ApplyLegacyDoAirComboAfterBeginGuards(const TCHAR* BranchLabelForLog)
{
	if (DoAirComboDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] Air — DoAction 행 없음 — %s"),
			*GetNameSafe(this)));
		return false;
	}

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] %s — StateComponent 미해결"),
			BranchLabelForLog));
		return false;
	}

	const bool bAllowByState = StateComp->IsIdle() || StateComp->IsHitted();
	if (false == bAllowByState)
	{
		return false;
	}

	StateComp->SetAction();

	ACPlayableCharacter* Playable = Cast<ACPlayableCharacter>(Character);
	if (IsValid(Playable))
	{
		Playable->SetCoordBeforeAir();
		Playable->SetFlyToFall(false);
	}

	const int32 Idx = FMath::Clamp(ComboHitIndexAir, 0, DoAirComboDatas.Num() - 1);
	PlayAction(DoAirComboDatas, Idx, FVector::ZeroVector);
	return true;
}

bool ACWeaponComboSkillContext::Combo_ApplyLegacyDoFlyingAttackAfterBeginGuards(const TCHAR* BranchLabelForLog)
{
	if (DoFlyingAttackDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] Flying — DoAction 행 없음 — %s"),
			*GetNameSafe(this)));
		return false;
	}

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] %s — StateComponent 미해결"),
			BranchLabelForLog));
		return false;
	}

	const bool bAllowByState = StateComp->IsMovementHittedState() || StateComp->IsFlying()
		|| StateComp->IsFalling();
	if (false == bAllowByState)
	{
		return false;
	}

	StateComp->SetAction();

	ACharacter* CharActor = Cast<ACharacter>(Character);
	UCharacterMovementComponent* MoveComp =
		IsValid(CharActor) ? CharActor->GetCharacterMovement() : nullptr;
	if (false == IsValid(MoveComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] %s — CharacterMovement 없음"),
			BranchLabelForLog));
		return false;
	}

	MoveComp->SetMovementMode(MOVE_Flying);

	const int32 Idx = FMath::Clamp(ComboHitIndexFlying, 0, DoFlyingAttackDatas.Num() - 1);
	PlayAction(DoFlyingAttackDatas, Idx, FVector::ZeroVector);
	return true;
}

void ACWeaponComboSkillContext::FallDownGravity()
{
	// BP: `Character->GetComponentByClass(MovingComponent)` 유효 시에만 `SetGravity(20)`.
	UCMovementComponent* MoveResolved = Moving;
	if (false == IsValid(MoveResolved) && IsValid(Character))
	{
		MoveResolved = Character->FindComponentByClass<UCMovementComponent>();
	}

	if (false == IsValid(MoveResolved))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] FallDownGravity: MovingComponent 미해결 — 중력 생략 — %s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
		return;
	}

	MoveResolved->SetGravity(ComboFallDownGravityBpGravityScale);
}

bool ACWeaponComboSkillContext::Combo_ApplyLegacyDoDownAttackAfterBeginGuards(const TCHAR* BranchLabelForLog)
{
	if (DoDownAttackDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] Down — DoAction 행 없음 — %s"),
			*GetNameSafe(this)));
		return false;
	}

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] %s — StateComponent 미해결"),
			BranchLabelForLog));
		return false;
	}

	const bool bAllowByState = StateComp->IsIdle() || StateComp->IsHitted();
	if (false == bAllowByState)
	{
		return false;
	}

	StateComp->SetAction();

	const int32 Idx = FMath::Clamp(ComboHitIndexDown, 0, DoDownAttackDatas.Num() - 1);
	PlayAction(DoDownAttackDatas, Idx, FVector::ZeroVector);
	return true;
}

bool ACWeaponComboSkillContext::Combo_ApplyLegacyDoDashAttackAfterBeginGuards(const TCHAR* BranchLabelForLog)
{
	if (DoSwayingAttackDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] DashAttack — DoAction 행 없음 — %s"),
			*GetNameSafe(this)));
		return false;
	}

	InGroundDashing = false;
	InAirDashing = false;

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] %s — StateComponent 미해결"),
			BranchLabelForLog));
		return false;
	}

	const bool bAllowByState = StateComp->IsIdle() || StateComp->IsHitted();
	if (false == bAllowByState)
	{
		return false;
	}

	StateComp->SetAction();

	const int32 Idx = FMath::Clamp(ComboHitIndexSwaying, 0, DoSwayingAttackDatas.Num() - 1);
	PlayAction(DoSwayingAttackDatas, Idx, FVector::ZeroVector);
	return true;
}

bool ACWeaponComboSkillContext::Combo_ApplyLegacyDoFallDownAttackAfterBeginGuards(const TCHAR* BranchLabelForLog)
{
	if (DoFallDownAttackDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] FallDown — DoAction 행 없음 — %s"),
			*GetNameSafe(this)));
		return false;
	}

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] %s — StateComponent 미해결"),
			BranchLabelForLog));
		return false;
	}

	const bool bAllowByState = StateComp->IsFalling() || StateComp->IsFlying();
	if (false == bAllowByState)
	{
		return false;
	}

	StateComp->SetAction();

	UWorld* World = GetWorld();
	if (false == IsValid(World))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] %s — World 없어 FallDown 타머 생략 — %s"),
			BranchLabelForLog,
			*GetNameSafe(this)));
	}
	else
	{
		World->GetTimerManager().ClearTimer(ComboFallDownGravityTimerHandle);
		World->GetTimerManager().SetTimer(
			ComboFallDownGravityTimerHandle,
			FTimerDelegate::CreateUObject(this, &ACWeaponComboSkillContext::FallDownGravity),
			ComboFallDownGravityDelaySeconds,
			false);
	}

	const int32 Idx = FMath::Clamp(ComboHitIndexFallDown, 0, DoFallDownAttackDatas.Num() - 1);
	PlayAction(DoFallDownAttackDatas, Idx, FVector::ZeroVector);
	return true;
}

bool ACWeaponComboSkillContext::Combo_ApplyLegacySkillAfterBeginGuards(const int32 InSkillIndex, const TCHAR* BranchLabelForLog)
{
	if (false == SkillDatas.IsValidIndex(InSkillIndex))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] Skill — 스킬 행 인덱스 무효(Index=%d Count=%d) — %s"),
			InSkillIndex,
			SkillDatas.Num(),
			*GetNameSafe(this)));
		return false;
	}

	ComboHitIndexSkill = InSkillIndex;

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] %s — StateComponent 미해결"),
			BranchLabelForLog));
		return false;
	}

	const bool bRidingBranch = StateComp->IsRiding();

	const bool bTimedGravityBranch = StateComp->IsIdle() || StateComp->IsMovementHittedState()
		|| StateComp->IsFlying() || StateComp->IsFalling();

	if (false == bRidingBranch && false == bTimedGravityBranch)
	{
		return false;
	}

	StateComp->SetAction();

	UWorld* World = GetWorld();
	const int32 PlayIndex = FMath::Clamp(InSkillIndex, 0, SkillDatas.Num() - 1);

	if (true == bTimedGravityBranch)
	{
		if (false == IsValid(World))
		{
			CLog::Log(FString::Printf(
				TEXT("[WeaponComboSkillContext] %s — World 없어 Skill용 FallDownGravity 타머 생략 — %s"),
				BranchLabelForLog,
				*GetNameSafe(this)));
		}
		else
		{
			World->GetTimerManager().ClearTimer(ComboFallDownGravityTimerHandle);
			World->GetTimerManager().SetTimer(
				ComboFallDownGravityTimerHandle,
				FTimerDelegate::CreateUObject(this, &ACWeaponComboSkillContext::FallDownGravity),
				ComboFallDownGravityDelaySeconds,
				false);
		}
	}

	PlayAction(SkillDatas, PlayIndex, FVector::ZeroVector);
	return true;
}

bool ACWeaponComboSkillContext::Combo_ApplyLegacyRidingAttackAfterBeginGuards(const TCHAR* BranchLabelForLog)
{
	if (DoActionDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] RidingAttack — DoAction(공통) 행 없음 — %s"),
			*GetNameSafe(this)));
		return false;
	}

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] %s — StateComponent 미해결"),
			BranchLabelForLog));
		return false;
	}

	if (false == StateComp->IsRiding())
	{
		return false;
	}

	StateComp->SetAction();

	const int32 Idx = FMath::Clamp(ComboHitIndexCommon, 0, DoActionDatas.Num() - 1);
	PlayAction(DoActionDatas, Idx, FVector::ZeroVector);
	return true;
}

void ACWeaponComboSkillContext::Combo_BeginFlyingSegmentFromNotify()
{
	if (false == InEquip || false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] BeginFlyingSegment: 장착/캐릭터 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	if (true == ComboExist)
	{
		ComboExist = false;
	}

	ComboHitIndexFlying += 1;

	if (DoFlyingAttackDatas.Num() <= 0)
	{
		return;
	}

	// 레거시 BP `Less` 분기와 동등: 증분한 인덱스가 행 배열 길이 미만일 때만 중력 보정·재생(True).
	if (ComboHitIndexFlying >= DoFlyingAttackDatas.Num())
	{
		return;
	}

	if (IsValid(Moving))
	{
		Moving->AddGravity(ComboBeginFlyingNotifyAddGravityFactor);
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] BeginFlyingSegment: Moving 없어 AddGravity 생략 — %s"),
			*GetNameSafe(this)));
	}

	PlayAction(DoFlyingAttackDatas, ComboHitIndexFlying, FVector::ZeroVector);
}

void ACWeaponComboSkillContext::Combo_PlayDatasOrWarn(
	const TArray<FDoActionData>& InDatas,
	const int32 InPreferredIndex,
	const TCHAR* BranchLabelForLog)
{
	if (InDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] %s — DoAction 행 없음 — %s"),
			BranchLabelForLog,
			*GetNameSafe(this)));
		return;
	}

	const int32 ClampedIndex = FMath::Clamp(InPreferredIndex, 0, InDatas.Num() - 1);
	PlayAction(InDatas, ClampedIndex, FVector::ZeroVector);
}

void ACWeaponComboSkillContext::Combo_DispatchDoAction(const CEAttackType InAttackType, const int32 InSkillIndex)
{
	switch (InAttackType)
	{
	case CEAttackType::Common:
		Combo_PlayDatasOrWarn(DoActionDatas, 0, TEXT("Common"));
		break;
	case CEAttackType::Air:
		(void)Combo_ApplyLegacyDoAirComboAfterBeginGuards(TEXT("Air"));
		break;
	case CEAttackType::Flying:
		(void)Combo_ApplyLegacyDoFlyingAttackAfterBeginGuards(TEXT("Flying"));
		break;
	case CEAttackType::Down:
		(void)Combo_ApplyLegacyDoDownAttackAfterBeginGuards(TEXT("Down"));
		break;
	case CEAttackType::DashAttack:
		(void)Combo_ApplyLegacyDoDashAttackAfterBeginGuards(TEXT("DashAttack"));
		break;
	case CEAttackType::FallDown:
		(void)Combo_ApplyLegacyDoFallDownAttackAfterBeginGuards(TEXT("FallDown"));
		break;
	case CEAttackType::Dash:
	case CEAttackType::AirDash:
		// 레거시 Weapon_Combo DoAction 스위치에서는 미연결 — 지면·공중 대시는 Dash()/AirDash() 가 PlayAction 처리.
		break;
	case CEAttackType::Skill:
		(void)Combo_ApplyLegacySkillAfterBeginGuards(InSkillIndex, TEXT("Skill"));
		break;
	case CEAttackType::Riding:
		(void)Combo_ApplyLegacyRidingAttackAfterBeginGuards(TEXT("RidingAttack"));
		break;
	case CEAttackType::Max:
	default:
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] 알 수 없는 AttackType=%d — %s"),
			static_cast<int32>(InAttackType),
			*GetNameSafe(this)));
		break;
	}
}

void ACWeaponComboSkillContext::ComboFork_Begin_Common()
{
}

void ACWeaponComboSkillContext::ComboFork_Begin_Air()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_Begin_Air: Character 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	ACharacter* CharActor = Cast<ACharacter>(Character);
	if (false == IsValid(CharActor))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_Begin_Air: ACharacter 아님 — %s"),
			*GetNameSafe(Character)));
		return;
	}

	const FVector LaunchVelocity(0.0, 0.0, ComboBeginAirLaunchUp);
	CharActor->LaunchCharacter(LaunchVelocity, false, false);

	UCharacterMovementComponent* MoveComp = CharActor->GetCharacterMovement();
	if (IsValid(MoveComp))
	{
		MoveComp->SetMovementMode(MOVE_Falling);
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (IsValid(StateComp))
	{
		StateComp->SetFalling();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_Begin_Air: StateComponent 없음 — %s"),
			*GetNameSafe(this)));
	}
}

void ACWeaponComboSkillContext::ComboFork_Begin_Flying()
{
	// 레거시 `Begin_DoFlyingAttack` 본문은 애님 노티 등에서 `Combo_BeginFlyingSegmentFromNotify` 로 분리.
}

void ACWeaponComboSkillContext::ComboFork_End_Air()
{
	if (IsValid(Moving))
	{
		Moving->UnFixCamera();
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (IsValid(StateComp))
	{
		StateComp->SetFalling();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_Air: StateComponent 없음 — %s"),
			*GetNameSafe(this)));
	}
}

void ACWeaponComboSkillContext::ComboFork_End_Common()
{
}

void ACWeaponComboSkillContext::ComboFork_End_Flying()
{
}

void ACWeaponComboSkillContext::ComboFork_End_Down()
{
	if (IsValid(Moving))
	{
		Moving->UnFixCamera();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_Down: Moving 없어 UnFixCamera 생략 — %s"),
			*GetNameSafe(this)));
	}
}

void ACWeaponComboSkillContext::ComboFork_End_DashAttack()
{
	if (IsValid(Moving))
	{
		Moving->UnFixCamera();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_DashAttack: Moving 없어 UnFixCamera 생략 — %s"),
			*GetNameSafe(this)));
	}
}

void ACWeaponComboSkillContext::ComboFork_End_FallDown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboFallDownGravityTimerHandle);
	}

	if (IsValid(Moving))
	{
		Moving->UnFixCamera();
		Moving->Move();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_FallDown: Moving 없어 UnFixCamera/Move 생략 — %s"),
			*GetNameSafe(this)));
	}

	ACharacter* CharActor = Cast<ACharacter>(Character);
	UCharacterMovementComponent* MoveComp = IsValid(CharActor) ? CharActor->GetCharacterMovement() : nullptr;
	if (false == IsValid(MoveComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_FallDown: CharacterMovement 미해결 — %s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
		return;
	}

	MoveComp->SetMovementMode(MOVE_Falling);
}

void ACWeaponComboSkillContext::ComboFork_End_Skill()
{
	// 레거시 `End_Skill` — 타이머로 남은 `FallDownGravity` 가 이후에 터지면 중력이 꼬일 수 있어 무효화.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboFallDownGravityTimerHandle);
	}

	if (IsValid(Moving))
	{
		Moving->UnFixCamera();
		Moving->Move();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_Skill: Moving 없어 UnFixCamera/Move 생략 — %s"),
			*GetNameSafe(this)));
	}

	ACharacter* CharActor = Cast<ACharacter>(Character);
	UCharacterMovementComponent* MoveComp = IsValid(CharActor) ? CharActor->GetCharacterMovement() : nullptr;
	if (false == IsValid(MoveComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_Skill: CharacterMovement 미해결 — %s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
		return;
	}

	MoveComp->SetMovementMode(MOVE_Falling);
}

void ACWeaponComboSkillContext::ComboFork_End_Riding()
{
	// 레거시 `End_RidingAttack` — 라이딩 콤보 인덱스를 0으로 초기화한 뒤 카메라만 복구한다.
	ComboHitIndexCommon = 0;

	if (IsValid(Moving))
	{
		Moving->UnFixCamera();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_Riding: Moving 없어 UnFixCamera 생략 — %s"),
			*GetNameSafe(this)));
	}
}

ACSkillWeapon::ACSkillWeapon()
{
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	PrimaryActorTick.bCanEverTick = false;
}

void ACSkillWeapon::BeginPlay()
{
	Super::BeginPlay();

	const TObjectPtr<AActor> ownerActor = GetOwner();
	if (false == IsValid(ownerActor))
	{
		CLog::Log(FString::Printf(
			TEXT("[SkillWeapon] BeginPlay: Owner 없음 — ChildActorComponent 또는 Spawn 시 Owner 미설정 — %s"),
			*GetName()));
		return;
	}

	// BP: Owner → Magic 이면 Magic 변수·Magic.Character 할당.
	if (ACMagicSkillContext* magicActor = Cast<ACMagicSkillContext>(ownerActor))
	{
		Magic = magicActor;
		Weapon = nullptr;
		Character = magicActor->Character;
		if (false == IsValid(Character))
			CLog::Log(FString::Printf(TEXT("[SkillWeapon] Magic 컨텍스트의 Character 미설정 — %s"), *GetName()));
		return;
	}

	// BP: Magic 실패 시 Owner → Weapon_C.
	if (ACWeaponSkillContext* weaponActor = Cast<ACWeaponSkillContext>(ownerActor))
	{
		Weapon = weaponActor;
		Magic = nullptr;
		Character = weaponActor->Character;
		if (false == IsValid(Character))
			CLog::Log(FString::Printf(TEXT("[SkillWeapon] Weapon 컨텍스트의 Character 미설정 — %s"), *GetName()));
		return;
	}

	// 애착 액터만 C++ 로 둔 경우: ACAttachment::Owner 로 동일 정보를 채운다.
	if (ACAttachment* attachment = Cast<ACAttachment>(ownerActor))
	{
		Magic = nullptr;
		Weapon = nullptr;
		Character = attachment->Owner.Get();
		if (false == IsValid(Character))
			CLog::Log(
				FString::Printf(TEXT("[SkillWeapon] ACAttachment Owner 미설정 — %s"), *GetName()));
		return;
	}

	CLog::Log(FString::Printf(
		TEXT("[SkillWeapon] Owner 타입 불일치 — Magic/Weapon 마커 또는 ACAttachment 가 아님 — Skill=%s Owner=%s"),
		*GetName(),
		*ownerActor->GetName()));
}
