#include "Components/CMagicComponent.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UCMagicComponent::UCMagicComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

int32 UCMagicComponent::SlotCountCeiling()
{
	return static_cast<int32>(CEMagicType::Max);
}

void UCMagicComponent::BeginPlay()
{
	Super::BeginPlay();

	SpawnMagicActorsAndHideAll();
	ApplyMagicActorsVisibilityFromSlot();
}

void UCMagicComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (int32 spawnIndex = 0; spawnIndex < Magics.Num(); ++spawnIndex)
	{
		if (IsValid(Magics[spawnIndex].Get()))
			Magics[spawnIndex]->Destroy();

		Magics[spawnIndex] = nullptr;
	}

	Magics.Empty();
	ActiveMagic = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UCMagicComponent::SpawnMagicActorsAndHideAll()
{
	AActor* owningActor = GetOwner();
	CheckNull(owningActor);

	ACCommonCharacter* commonOwner = Cast<ACCommonCharacter>(owningActor);
	UWorld* world = owningActor->GetWorld();
	CheckNull(world);

	const int32 maxSlotsLocal = SlotCountCeiling();

	Magics.SetNum(maxSlotsLocal);
	for (int32 spawnIndexLocal = 0; spawnIndexLocal < maxSlotsLocal; ++spawnIndexLocal)
	{
		Magics[spawnIndexLocal] = nullptr;
		if (spawnIndexLocal >= MagicClasses.Num())
			continue;

		const TSubclassOf<ACMagicSkillContext> subclass = MagicClasses[spawnIndexLocal];
		if (nullptr == subclass.Get())
			continue;

		FActorSpawnParameters spawnParameters;
		spawnParameters.Owner = owningActor;
		spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ACMagicSkillContext* spawnedMagic = world->SpawnActor<ACMagicSkillContext>(
			subclass.Get(), FVector::ZeroVector, FRotator::ZeroRotator, spawnParameters);

		if (false == IsValid(spawnedMagic))
		{
			CLog::Log(FString::Printf(TEXT("[MagicComponent] Magic 스폰 실패 슬롯=%d 클래스=%s — 소유자=%s"),
				spawnIndexLocal, *subclass->GetName(), *owningActor->GetName()));
			continue;
		}

		spawnedMagic->Character = commonOwner;

		if (AttachSocketName.IsNone())
			spawnedMagic->AttachToActor(owningActor, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		else
		{
			ACharacter* asCharacter = Cast<ACharacter>(owningActor);
			USkeletalMeshComponent* mesh = (IsValid(asCharacter)) ? asCharacter->GetMesh() : nullptr;
			if (IsValid(mesh))
			{
				spawnedMagic->AttachToComponent(mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocketName);
			}
			else
			{
				spawnedMagic->AttachToActor(owningActor, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
		}

		spawnedMagic->SetActorHiddenInGame(true);
		Magics[spawnIndexLocal] = spawnedMagic;
	}
}

void UCMagicComponent::CommitMagicTypeSlotApplyVisibilityMulticast(
	CEMagicType InNextMagicType,
	CEMagicType InDelegatePrevMagicType)
{
	MagicTypeSlot = InNextMagicType;
	ApplyMagicActorsVisibilityFromSlot();

	if (OnMagicTypeChanged.IsBound())
		OnMagicTypeChanged.Broadcast(InNextMagicType, InDelegatePrevMagicType);
}

bool UCMagicComponent::IsUnarmed() const
{
	return MagicTypeSlot == CEMagicType::Unarmed;
}

void UCMagicComponent::SetUnarmed()
{
	if (IsValid(ActiveMagic.Get()))
		ActiveMagic->Unequip();

	ChangeType(CEMagicType::Unarmed);
}

void UCMagicComponent::SetWarp()
{
	SetMode(CEMagicType::Warp);
}

void UCMagicComponent::SetAround()
{
	SetMode(CEMagicType::Around);
}

void UCMagicComponent::SetFireBall()
{
	SetMode(CEMagicType::FireBall);
}

void UCMagicComponent::SetBomb()
{
	SetMode(CEMagicType::Bomb);
}

void UCMagicComponent::SetYondu()
{
	SetMode(CEMagicType::Yondu);
}

void UCMagicComponent::SetMode(CEMagicType InType)
{
	if (MagicTypeSlot == InType)
	{
		// 같은 모드 재선택 시 맨손으로 전환한다.
		SetUnarmed();
		return;
	}

	if ((MagicTypeSlot != CEMagicType::Unarmed) && IsValid(ActiveMagic.Get()))
		ActiveMagic->Unequip();

	if (InType != CEMagicType::Unarmed)
	{
		const int32 slotChosenIndex = static_cast<int32>(InType);
		if (Magics.IsValidIndex(slotChosenIndex))
		{
			ACMagicSkillContext* equipTargetMagic = Magics[slotChosenIndex].Get();
			if (IsValid(equipTargetMagic))
				equipTargetMagic->Equip();
		}
	}

	const CEMagicType previousMagicGameplayType = MagicTypeSlot;
	CommitMagicTypeSlotApplyVisibilityMulticast(InType, previousMagicGameplayType);
}

void UCMagicComponent::ChangeType(CEMagicType InType)
{
	const CEMagicType previousMagicGameplayType = MagicTypeSlot;
	CommitMagicTypeSlotApplyVisibilityMulticast(InType, previousMagicGameplayType);
}

void UCMagicComponent::GetType(CEMagicType& OutMainType)
{
	OutMainType = MagicTypeSlot;
}

void UCMagicComponent::Begin_Equip()
{
	// Unarmed 또는 슬롯 미설정 상태에서 장착 애니만 도는 경우 흔하므로 로그 없이 무시한다.
	if (false == IsValid(ActiveMagic.Get()))
		return;
	ActiveMagic->Begin_Equip();
}

void UCMagicComponent::End_Equip()
{
	if (false == IsValid(ActiveMagic.Get()))
		return;
	ActiveMagic->End_Equip();
}

void UCMagicComponent::Begin_Unequip()
{
	if (false == IsValid(ActiveMagic.Get()))
		return;
	ActiveMagic->Begin_Unequip();
}

void UCMagicComponent::End_Unequip()
{
	if (false == IsValid(ActiveMagic.Get()))
		return;
	ActiveMagic->End_Unequip();
}

void UCMagicComponent::DoAction(CEAttackType InAttackType, int32 InSkillIndex)
{
	if (false == IsValid(ActiveMagic.Get()))
		return;
	ActiveMagic->DoAction(InAttackType, InSkillIndex);
}

void UCMagicComponent::Begin_DoAction(CEAttackType InAttackType)
{
	if (false == IsValid(ActiveMagic.Get()))
		return;
	ActiveMagic->Begin_DoAction(InAttackType);
}

void UCMagicComponent::End_DoAction(CEAttackType InAttackType)
{
	if (false == IsValid(ActiveMagic.Get()))
		return;
	ActiveMagic->End_DoAction(InAttackType);
}

void UCMagicComponent::ApplyMagicActorsVisibilityFromSlot()
{
	const int32 chosenSlotLocal = static_cast<int32>(MagicTypeSlot);

	ActiveMagic = nullptr;
	const int32 lastIndexLocal = Magics.Num();

	for (int32 hideIndexLocal = 0; hideIndexLocal < lastIndexLocal; ++hideIndexLocal)
	{
		ACMagicSkillContext* eachMagic = Magics[hideIndexLocal].Get();
		if (false == IsValid(eachMagic))
			continue;

		eachMagic->SetActorHiddenInGame(true);
	}

	if (MagicTypeSlot == CEMagicType::Unarmed)
		return;

	if ((false == Magics.IsValidIndex(chosenSlotLocal)) || false == IsValid(Magics[chosenSlotLocal].Get()))
		return;

	ACMagicSkillContext* visibleMagic = Magics[chosenSlotLocal].Get();
	visibleMagic->SetActorHiddenInGame(false);
	ActiveMagic = visibleMagic;
}

void UCMagicComponent::SyncEquippedMagicAfterWeaponPipeline(CEMagicType CurrentMagic, CEMagicType PreviousMagic)
{
	if (MagicTypeSlot == CurrentMagic)
		return;

	// SetMode 와 같은 Equip/Unequip 순서만, 동일 타입 재선택 토글은 무기측에서 처리하지 않는다.
	if ((MagicTypeSlot != CEMagicType::Unarmed) && IsValid(ActiveMagic.Get()))
		ActiveMagic->Unequip();

	if (CurrentMagic != CEMagicType::Unarmed)
	{
		const int32 weaponSyncSlotChosenIndex = static_cast<int32>(CurrentMagic);
		if (Magics.IsValidIndex(weaponSyncSlotChosenIndex))
		{
			ACMagicSkillContext* weaponSyncEquipTargetMagic = Magics[weaponSyncSlotChosenIndex].Get();
			if (IsValid(weaponSyncEquipTargetMagic))
				weaponSyncEquipTargetMagic->Equip();
		}
	}

	CommitMagicTypeSlotApplyVisibilityMulticast(CurrentMagic, PreviousMagic);
}
