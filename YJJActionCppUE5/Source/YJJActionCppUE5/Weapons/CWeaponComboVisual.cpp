#include "Weapons/CWeaponComboVisual.h"

#include "Characters/CCommonCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Utilities/CLog.h"

namespace
{
	template<typename TComponent>
	TComponent* ComboEquip_FindComponentByName(AActor* Owner, const FName PreferredName)
	{
		if (false == IsValid(Owner))
		{
			return nullptr;
		}

		if (false == PreferredName.IsNone())
		{
			if (TComponent* Subobject = Cast<TComponent>(Owner->GetDefaultSubobjectByName(PreferredName)))
			{
				return Subobject;
			}

			TArray<TComponent*> Components;
			Owner->GetComponents<TComponent>(Components, true);
			for (TComponent* Component : Components)
			{
				if (Component->GetFName() == PreferredName)
				{
					return Component;
				}
			}
		}

		return Owner->FindComponentByClass<TComponent>();
	}

	USceneComponent* ComboEquip_FindSceneComponentByName(AActor* Owner, const FName PreferredName)
	{
		if (false == IsValid(Owner))
		{
			return nullptr;
		}

		if (false == PreferredName.IsNone())
		{
			if (USceneComponent* Subobject = Cast<USceneComponent>(Owner->GetDefaultSubobjectByName(PreferredName)))
			{
				return Subobject;
			}

			TArray<USceneComponent*> Components;
			Owner->GetComponents<USceneComponent>(Components, true);
			for (USceneComponent* Component : Components)
			{
				if (Component->GetFName() == PreferredName)
				{
					return Component;
				}
			}
		}

		return Cast<USceneComponent>(Owner->GetRootComponent());
	}

	void ComboEquip_SetSceneVisibility(USceneComponent* SceneComp, const bool bVisible)
	{
		if (IsValid(SceneComp))
		{
			SceneComp->SetVisibility(bVisible, false);
		}
	}

	void ComboEquip_AttachSceneToCharacterMesh(
		USceneComponent* SceneComp,
		USkeletalMeshComponent* CharMesh,
		const FName SocketName)
	{
		if (false == IsValid(SceneComp) || false == IsValid(CharMesh))
		{
			return;
		}

		SceneComp->AttachToComponent(
			CharMesh,
			FAttachmentTransformRules::KeepRelativeTransform,
			SocketName);
	}

	void ComboEquip_DetachScene(USceneComponent* SceneComp)
	{
		if (IsValid(SceneComp))
		{
			SceneComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		}
	}
}

ACWeaponComboVisual::ACWeaponComboVisual()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACWeaponComboVisual::BeginPlay()
{
	Super::BeginPlay();

	if (EquipVisualMode == CEComboEquipVisualMode::None)
	{
		return;
	}

	ComboEquip_ResolveComponents();
	ComboEquip_ApplyBeginPlayVisualState();
}

void ACWeaponComboVisual::Unequip_Implementation()
{
	ComboEquip_ApplyUnequipVisualState();
	Super::Unequip_Implementation();
}

void ACWeaponComboVisual::Begin_Equip_Implementation(const bool bMainOrSubWeapon)
{
	Super::Begin_Equip_Implementation(bMainOrSubWeapon);
	ComboEquip_ApplyBeginEquipVisualState();
}

void ACWeaponComboVisual::Skill(const int32 InSkillIndex)
{
	(void)InSkillIndex;

	if (EquipVisualMode == CEComboEquipVisualMode::DualWield)
	{
		ComboEquip_AttachFallDownCapsule();
	}
}

void ACWeaponComboVisual::End_Skill()
{
	if (EquipVisualMode == CEComboEquipVisualMode::DualWield)
	{
		ComboEquip_DetachFallDownCapsule();
	}
}

void ACWeaponComboVisual::Host_Skill(const int32 InSkillIndex)
{
	Skill(InSkillIndex);
}

void ACWeaponComboVisual::Host_EndSkill()
{
	End_Skill();
}

void ACWeaponComboVisual::DoFallDownAttack()
{
	if (EquipVisualMode == CEComboEquipVisualMode::DualWield)
	{
		ComboEquip_AttachFallDownCapsule();
	}
}

void ACWeaponComboVisual::End_DoFallDownAttack()
{
	if (EquipVisualMode == CEComboEquipVisualMode::DualWield)
	{
		ComboEquip_DetachFallDownCapsule();
	}
}

void ACWeaponComboVisual::Weapon_DoActionImpl(const CEAttackType InAttackType, const int32 InSkillIndex)
{
	Super::Weapon_DoActionImpl(InAttackType, InSkillIndex);

	if (EquipVisualMode != CEComboEquipVisualMode::DualWield)
	{
		return;
	}

	if (InAttackType == CEAttackType::FallDown || InAttackType == CEAttackType::Skill)
	{
		ComboEquip_AttachFallDownCapsule();
	}
}

void ACWeaponComboVisual::ComboFork_End_FallDown()
{
	if (EquipVisualMode == CEComboEquipVisualMode::DualWield)
	{
		ComboEquip_DetachFallDownCapsule();
	}

	Super::ComboFork_End_FallDown();
}

void ACWeaponComboVisual::ComboFork_End_Skill()
{
	if (EquipVisualMode == CEComboEquipVisualMode::DualWield)
	{
		ComboEquip_DetachFallDownCapsule();
	}

	Super::ComboFork_End_Skill();
}

void ACWeaponComboVisual::ComboEquip_ResolveComponents()
{
	switch (EquipVisualMode)
	{
	case CEComboEquipVisualMode::SingleMeshOnEquip:
		ResolvedSingleMesh = ComboEquip_FindSceneComponentByName(this, SingleMeshComponentName);
		if (false == IsValid(ResolvedSingleMesh))
		{
			ResolvedSingleMesh = ComboEquip_FindComponentByName<USkeletalMeshComponent>(this, SingleMeshComponentName);
		}
		if (false == IsValid(ResolvedSingleMesh))
		{
			CLog::Log(FString::Printf(
				TEXT("[WeaponComboVisual] SingleMeshOnEquip — 메시 컴포넌트 없음 — %s"),
				*GetNameSafe(this)));
		}
		break;

	case CEComboEquipVisualMode::SingleMeshRootOnBeginPlay:
		ResolvedSingleMesh = ComboEquip_FindSceneComponentByName(this, SingleMeshComponentName);
		if (false == IsValid(ResolvedSingleMesh))
		{
			ResolvedSingleMesh = ComboEquip_FindComponentByName<UStaticMeshComponent>(this, SingleMeshComponentName);
		}
		if (false == IsValid(ResolvedSingleMesh))
		{
			CLog::Log(FString::Printf(
				TEXT("[WeaponComboVisual] SingleMeshRootOnBeginPlay — StaticMesh 없음 — %s"),
				*GetNameSafe(this)));
		}
		break;

	case CEComboEquipVisualMode::DualWield:
		ResolvedLeftMesh = ComboEquip_FindSceneComponentByName(this, LeftMeshComponentName);
		ResolvedRightMesh = ComboEquip_FindSceneComponentByName(this, RightMeshComponentName);
		ResolvedLeftCapsule = ComboEquip_FindSceneComponentByName(this, LeftCapsuleComponentName);
		ResolvedRightCapsule = ComboEquip_FindSceneComponentByName(this, RightCapsuleComponentName);
		ResolvedFallDownCapsule = ComboEquip_FindSceneComponentByName(this, FallDownCapsuleComponentName);
		break;

	default:
		break;
	}
}

USkeletalMeshComponent* ACWeaponComboVisual::ComboEquip_ResolveOwnerCharacterMesh() const
{
	if (false == IsValid(Character))
	{
		return nullptr;
	}

	const ACharacter* CharActor = Cast<ACharacter>(Character);
	if (false == IsValid(CharActor))
	{
		return nullptr;
	}

	return CharActor->GetMesh();
}

void ACWeaponComboVisual::ComboEquip_ApplyBeginPlayVisualState()
{
	switch (EquipVisualMode)
	{
	case CEComboEquipVisualMode::SingleMeshOnEquip:
		ComboEquip_SetSingleMeshVisible(false);
		break;

	case CEComboEquipVisualMode::SingleMeshRootOnBeginPlay:
		ComboEquip_AttachActorRootToCharacterHand();
		ComboEquip_SetSingleMeshVisible(false);
		break;

	case CEComboEquipVisualMode::DualWield:
		ComboEquip_ApplyDualHiddenAndDetachedState();
		break;

	default:
		break;
	}
}

void ACWeaponComboVisual::ComboEquip_ApplyUnequipVisualState()
{
	switch (EquipVisualMode)
	{
	case CEComboEquipVisualMode::SingleMeshOnEquip:
	case CEComboEquipVisualMode::SingleMeshRootOnBeginPlay:
		ComboEquip_SetSingleMeshVisible(false);
		break;

	case CEComboEquipVisualMode::DualWield:
		ComboEquip_ApplyDualHiddenAndDetachedState();
		ComboEquip_DetachFallDownCapsule();
		break;

	default:
		break;
	}
}

void ACWeaponComboVisual::ComboEquip_ApplyBeginEquipVisualState()
{
	switch (EquipVisualMode)
	{
	case CEComboEquipVisualMode::SingleMeshOnEquip:
		ComboEquip_SetSingleMeshVisible(true);
		ComboEquip_AttachSingleMeshToHand();
		break;

	case CEComboEquipVisualMode::SingleMeshRootOnBeginPlay:
		ComboEquip_SetSingleMeshVisible(true);
		break;

	case CEComboEquipVisualMode::DualWield:
		ComboEquip_ApplyDualVisibleAndAttachedState();
		break;

	default:
		break;
	}
}

void ACWeaponComboVisual::ComboEquip_AttachActorRootToCharacterHand()
{
	USceneComponent* Root = GetRootComponent();
	USkeletalMeshComponent* CharMesh = ComboEquip_ResolveOwnerCharacterMesh();
	if (false == IsValid(Root) || false == IsValid(CharMesh))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboVisual] Root/Character Mesh 없어 Root 부착 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	ComboEquip_AttachSceneToCharacterMesh(Root, CharMesh, HandAttachSocketName);
}

void ACWeaponComboVisual::ComboEquip_SetSingleMeshVisible(const bool bVisible)
{
	ComboEquip_SetSceneVisibility(ResolvedSingleMesh, bVisible);
}

void ACWeaponComboVisual::ComboEquip_AttachSingleMeshToHand()
{
	USkeletalMeshComponent* CharMesh = ComboEquip_ResolveOwnerCharacterMesh();
	if (false == IsValid(ResolvedSingleMesh) || false == IsValid(CharMesh))
	{
		return;
	}

	ComboEquip_AttachSceneToCharacterMesh(ResolvedSingleMesh, CharMesh, HandAttachSocketName);
}

void ACWeaponComboVisual::ComboEquip_ApplyDualHiddenAndDetachedState()
{
	ComboEquip_SetSceneVisibility(ResolvedLeftMesh, false);
	ComboEquip_SetSceneVisibility(ResolvedRightMesh, false);
	ComboEquip_DetachScene(ResolvedLeftCapsule);
	ComboEquip_DetachScene(ResolvedRightCapsule);
}

void ACWeaponComboVisual::ComboEquip_ApplyDualVisibleAndAttachedState()
{
	USkeletalMeshComponent* CharMesh = ComboEquip_ResolveOwnerCharacterMesh();
	if (false == IsValid(CharMesh))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboVisual] Dual 장착 — Character Mesh 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	ComboEquip_SetSceneVisibility(ResolvedLeftMesh, true);
	ComboEquip_SetSceneVisibility(ResolvedRightMesh, true);
	ComboEquip_AttachSceneToCharacterMesh(ResolvedLeftCapsule, CharMesh, LeftHandSocketName);
	ComboEquip_AttachSceneToCharacterMesh(ResolvedRightCapsule, CharMesh, RightHandSocketName);
}

void ACWeaponComboVisual::ComboEquip_AttachFallDownCapsule()
{
	if (EquipVisualMode != CEComboEquipVisualMode::DualWield)
	{
		return;
	}

	if (false == IsValid(ResolvedFallDownCapsule))
	{
		ResolvedFallDownCapsule = ComboEquip_FindSceneComponentByName(this, FallDownCapsuleComponentName);
	}

	USkeletalMeshComponent* CharMesh = ComboEquip_ResolveOwnerCharacterMesh();
	if (false == IsValid(ResolvedFallDownCapsule) || false == IsValid(CharMesh))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboVisual] FallDownCapsule 부착 실패 — %s"),
			*GetNameSafe(this)));
		return;
	}

	ComboEquip_AttachSceneToCharacterMesh(ResolvedFallDownCapsule, CharMesh, RightHandSocketName);
}

void ACWeaponComboVisual::ComboEquip_DetachFallDownCapsule()
{
	if (false == IsValid(ResolvedFallDownCapsule))
	{
		ResolvedFallDownCapsule = ComboEquip_FindSceneComponentByName(this, FallDownCapsuleComponentName);
	}

	ComboEquip_DetachScene(ResolvedFallDownCapsule);
}

ACWeaponComboSword::ACWeaponComboSword()
{
	EquipVisualMode = CEComboEquipVisualMode::SingleMeshOnEquip;
	HandAttachSocketName = TEXT("Hand_Sword");
	HolsterAttachSocketName = TEXT("Holster_Sword");
	SingleMeshComponentName = TEXT("SkeletalMesh");
}

ACWeaponComboHammer::ACWeaponComboHammer()
{
	EquipVisualMode = CEComboEquipVisualMode::SingleMeshRootOnBeginPlay;
	HandAttachSocketName = TEXT("Hand_Hammer");
	SingleMeshComponentName = TEXT("StaticMesh");
}

ACWeaponComboDual::ACWeaponComboDual()
{
	EquipVisualMode = CEComboEquipVisualMode::DualWield;
}
