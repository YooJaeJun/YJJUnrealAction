#include "Weapons/CMagic.h"

#include "Commons/CYJJBlueprintLibrary.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CTargetingComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CWeaponComponent.h"
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
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

void ACMagic::SetControllerInEquip()
{
	ACCommonCharacter* OwnerChar = Cast<ACCommonCharacter>(GetOwner());
	if (false == IsValid(OwnerChar))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] SetControllerInEquip(Owner 가 ACCommonCharacter 가 아님 — Controller 미갱신): 컨텍스트=%s Owner=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetOwner())));
		return;
	}

	// 레거시 BP: BP_Character.CurController → Magic.Controller (Character 멤버가 아닌 Owning 스택 기준으로 맞춤).
	Controller = OwnerChar->CurController.Get();
}

UCStateComponent* ACMagic::Magic_ResolveStateComponent() const
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

bool ACMagic::Magic_IsCharacterRealRiding() const
{
	UCStateComponent* stateCompResolved = Magic_ResolveStateComponent();
	if (false == IsValid(stateCompResolved))
	{
		// SetCandidate 류 간헐 호출 — Tick 아님, State UObject 미설정만 false 처리(레거시 그래프와 동일하게 조용히 실패).
		return false;
	}

	return stateCompResolved->IsRealRiding();
}

void ACMagic::Magic_RefreshStateReferenceFromCharacter()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] State 갱신: Character 없음 — 컨텍스트=%s Owner=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetOwner())));
		State = nullptr;
		return;
	}

	UCStateComponent* resolvedState = Character->FindComponentByClass<UCStateComponent>();
	State = resolvedState;

	if (false == IsValid(resolvedState))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] State 갱신: UCStateComponent 없음 — Character=%s 컨텍스트=%s"),
			*GetNameSafe(Character),
			*GetNameSafe(this)));
	}
}

UCMovementComponent* ACMagic::Magic_ResolveMovementForOwnerOrMount() const
{
	if (IsValid(Moving))
	{
		return Moving;
	}

	if (false == IsValid(Character))
	{
		return nullptr;
	}

	if (IsValid(Character->CurInteractingActor))
	{
		UCMovementComponent* fromInteract = Character->CurInteractingActor->FindComponentByClass<UCMovementComponent>();
		if (IsValid(fromInteract))
		{
			return fromInteract;
		}
	}

	return Character->FindComponentByClass<UCMovementComponent>();
}

void ACMagic::Magic_FixCameraForOwnerMovement()
{
	UCMovementComponent* moveResolved = Magic_ResolveMovementForOwnerOrMount();
	if (false == IsValid(moveResolved))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] FixCamera: UCMovementComponent 없음 — %s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
		return;
	}

	moveResolved->FixCamera();
}

void ACMagic::Magic_UnFixCameraForOwnerMovement()
{
	UCMovementComponent* moveResolved = Magic_ResolveMovementForOwnerOrMount();
	if (false == IsValid(moveResolved))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] UnFixCamera: UCMovementComponent 없음 — %s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
		return;
	}

	moveResolved->UnFixCamera();
}

void ACMagic::Magic_SetOwnerStateAction()
{
	UCStateComponent* stateCompResolved = Magic_ResolveStateComponent();
	if (false == IsValid(stateCompResolved))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] SetAction: UCStateComponent 없음 — %s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
		return;
	}

	stateCompResolved->SetAction();
}

bool ACMagic::Magic_IsOwnerIdle() const
{
	UCStateComponent* stateCompResolved = Magic_ResolveStateComponent();
	if (false == IsValid(stateCompResolved))
	{
		// DoAction 조건식에서 자주 조합됨 — State 미연결은 false 만 반환(로그 스팸 방지).
		return false;
	}

	return stateCompResolved->IsIdle();
}

bool ACMagic::Magic_IsOwnerRiding() const
{
	UCStateComponent* stateCompResolved = Magic_ResolveStateComponent();
	if (false == IsValid(stateCompResolved))
	{
		return false;
	}

	return stateCompResolved->IsRiding();
}

bool ACMagic::Magic_IsOwnerInAction() const
{
	UCStateComponent* stateCompResolved = Magic_ResolveStateComponent();
	if (false == IsValid(stateCompResolved))
	{
		return false;
	}

	return stateCompResolved->IsAction();
}

void ACMagic::Magic_ApplyEquipRowFacingAndMovement(const FEquipData& Row)
{
	if (false == IsValid(Moving))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] Equip 이동 분기 중 Moving 미연결 — %s"),
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

void ACMagic::Equip_Implementation()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] Equip(Character 없음): %s"),
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
			TEXT("[Magic] Equip: UCStateComponent 없음 — Character=%s 무기=%s"),
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

void ACMagic::Unequip_Implementation()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] Unequip(Character 없음): %s"),
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
			TEXT("[Magic] Unequip: Moving 미연결 — %s"),
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

void ACMagic::Begin_Equip_Implementation()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] Begin_Equip(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	IsEquipping = true;

	if (IsValid(EquipData.Sound))
	{
		EquipData.PlaySoundWave(TWeakObjectPtr<ACCommonCharacter>(Character));
	}
}

void ACMagic::End_Equip_Implementation()
{
	IsEquipping = false;
	InEquip = true;

	if (IsValid(Moving))
	{
		Moving->Move();
	}

	RestorePrevState();
}

void ACMagic::Begin_Unequip_Implementation()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] Begin_Unequip(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	IsUnequipping = true;

	if (IsValid(UnequipData.Sound))
	{
		UnequipData.PlaySoundWave(TWeakObjectPtr<ACCommonCharacter>(Character));
	}
}

void ACMagic::End_Unequip_Implementation()
{
	IsUnequipping = false;
	InEquip = false;

	if (IsValid(Moving))
	{
		Moving->Move();
	}
}

void ACMagic::DoAction_Implementation(CEAttackType InAttackType, int32 InSkillIndex)
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

void ACMagic::Begin_DoAction_Implementation(CEAttackType InAttackType)
{
}

void ACMagic::End_DoAction_Implementation(CEAttackType InAttackType)
{
	// 레거시 Magic End_DoAction BP 는 비었으나 애니 노티 종료 체인에서 호출된다 — 무기 End_DoAction 과 동등하게 상태·Moving 을 원복한다(UCMagicComponent 재진입 금지).
	InAction = false;

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] End_DoAction(Character 없음 — 이동·복구 생략): %s attack=%d"),
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

	// 레거시 Weapon::End_DoAction 과 동일 — UCAct::End_Act(SetIdle)까지 동기화. End_Action 만으로는 Act 고착이 남을 수 있다.
	if (UCWeaponComponent* weaponCompResolved = Character->FindComponentByClass<UCWeaponComponent>())
	{
		weaponCompResolved->End_DoAction(InAttackType);
	}

	(void)InAttackType;
}

ACMagic::ACMagic()
{
	// 레거시 Magic ReceiveTick — 탑승 시 Moving 을 상호작용 액터 쪽 컴포넌트로 갱신한다.
	PrimaryActorTick.bCanEverTick = true;
}

void ACMagic::BeginPlay()
{
	Super::BeginPlay();

	Magic_ClearRuntimeTableCaches();
	Magic_SetCharacterAndComponentsFromBlueprint();
	Magic_LoadEquipDatasFromTable();
	Magic_LoadDoActionDatasFromTable_CommonOnly();
	Magic_LoadHitDatasFromTable_CommonOnly();
}

void ACMagic::Magic_ClearRuntimeTableCaches()
{
	DoActionDatas.Reset();
	HitCommonDatas.Reset();
}

void ACMagic::Magic_SetCharacterAndComponentsFromBlueprint()
{
	Character = Cast<ACCommonCharacter>(GetOwner());

	if (false == IsValid(Character))
	{
		Moving = nullptr;
		State = nullptr;
		CLog::Log(FString::Printf(
			TEXT("[Magic] BeginPlay(Set Character 실패): Owner 가 ACCommonCharacter 가 아니거나 무효 — Actor=%s Owner=%s"),
			*GetName(),
			IsValid(GetOwner()) ? *GetOwner()->GetName() : TEXT("(null)")));
		return;
	}

	Moving = Character->FindComponentByClass<UCMovementComponent>();
	if (false == IsValid(Moving))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] 캐릭터 %s 에 UCMovementComponent 없음 — Moving 미연결."),
			*Character->GetName()));
	}

	State = Character->FindComponentByClass<UCStateComponent>();
}

void ACMagic::Magic_LoadEquipDatasFromTable()
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
				TEXT("[Magic] EquipDataTable '%s' 첫 행 '%s' 를 FEquipData 로 읽지 못했습니다."),
				*EquipDataTable->GetName(),
				*RowNames[0].ToString()));
	}

	if (RowCount > 1)
	{
		if (const FEquipData* UnequipRow = EquipDataTable->FindRow<FEquipData>(RowNames[1], TEXT("MagicEquip_Row1")))
			UnequipData = *UnequipRow;
		else
			CLog::Log(FString::Printf(
				TEXT("[Magic] EquipDataTable '%s' 둘째 행 '%s' 를 FEquipData 로 읽지 못했습니다."),
				*EquipDataTable->GetName(),
				*RowNames[1].ToString()));
	}
}

void ACMagic::Magic_LoadDoActionDatasFromTable_CommonOnly()
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

void ACMagic::Magic_LoadHitDatasFromTable_CommonOnly()
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

void ACMagic::Tick(const float DeltaSeconds)
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

void ACMagic::PlayAction(const TArray<FDoActionData>& InDatas, int32 InIndex)
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] PlayAction(Character 없음): %s Owner=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetOwner())));
		return;
	}

	if (false == InDatas.IsValidIndex(InIndex))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] PlayAction(인덱스 범위 밖 — %s): Index=%d Count=%d"),
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

void ACMagic::RestorePrevState()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] RestorePrevState(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	Character->ApplyRestoreStateFromPrevMode();
}

bool ACMagic::ConsumeMana(const double InMana)
{
	ACPlayableCharacter* PlayableResolve = Cast<ACPlayableCharacter>(Character);

	if (false == IsValid(PlayableResolve))
	{
		CLog::Log(FString::Printf(
			TEXT("[Magic] ConsumeMana(플레이어가 아님 — 실패 처리): 컨텍스트=%s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
		return false;
	}

	return PlayableResolve->ConsumeMana(InMana);
}

void ACMagic::Destroy_FromMagicBlueprint()
{
	Destroy();
}

void ACMagic::Magic_PlayActionRunExecutionBranches(
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
			TEXT("[Magic] PlayAction: Moving 무효 — Move/카메라 분기 생략 — %s"),
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

ACMagicAround::ACMagicAround()
{
}

void ACMagicAround::DoAction_Implementation(CEAttackType InAttackType, int32 InSkillIndex)
{
	(void)InSkillIndex;

	ACMagic::DoAction_Implementation(InAttackType, InSkillIndex);

	UCStateComponent* StateComp = Magic_ResolveStateComponent();
	const int32 RowCount = DoActionDatas.Num();
	if (RowCount <= 0)
	{
		return;
	}

	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicAround] DoAction: DoAction 행은 있으나 UCStateComponent 를 찾지 못함 — %s Rows=%d"),
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

void ACMagicAround::Begin_DoAction_Implementation(CEAttackType InAttackType)
{
	ACMagic::Begin_DoAction_Implementation(InAttackType);

	Around_SpawnRandomSkillActorFromSkillClasses();
}

void ACMagicAround::Around_SpawnRandomSkillActorFromSkillClasses()
{
	const int32 ClassCount = SkillClasses.Num();
	if (ClassCount <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicAround] SkillClasses 비어 있음 — 스폰 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	UWorld* World = GetWorld();
	if (false == IsValid(World))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicAround] World 무효 — 스폰 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	const int32 PickedIndex = FMath::RandRange(0, ClassCount - 1);
	TSubclassOf<AActor> PickClass = SkillClasses[PickedIndex];
	if (nullptr == PickClass)
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicAround] SkillClasses[%d] 무효 — 스폰 생략 — %s"),
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
			TEXT("[MagicAround] SpawnActor 실패 — Class=%s — %s"),
			*GetNameSafe(PickClass.Get()),
			*GetNameSafe(this)));
	}
}

