#include "Components/CWeaponComponent.h"
#include "Global.h"
#include "Components/CStateComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CEquipment.h"
#include "Weapons/CAct.h"

UCWeaponComponent::UCWeaponComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	CheckNull(Owner);

	constexpr uint8 size = static_cast<uint8>(EWeaponType::Max);

	for (int32 i = 0; i < size; i++)
	{
		if (!!DataAssets[i])
		{
			DataAssetsCopy[i] = NewObject<UCWeaponAsset>(
				this, 
				DataAssets[i]->GetClass(), 
				static_cast<FName>(DataAssets[i]->GetClass()->GetName()));

			DataAssetsCopy[i]->CopyDeep(*DataAssets[i], Owner);
		}
	}
}

void UCWeaponComponent::InputAction_Act()
{
	if (!!GetAct())
		GetAct()->Act();
}

void UCWeaponComponent::SetMode(EWeaponType InType)
{
	if (Type == InType)
	{
		SetUnarmedMode();
		return;
	}

	if (false == IsUnarmedMode())
	{
		CheckNull(GetEquipment());
		GetEquipment()->Unequip();
	}

	if (!!DataAssets[static_cast<uint8>(InType)])
	{
		const TWeakObjectPtr<UCEquipment> equipment = DataAssetsCopy[static_cast<uint8>(InType)]->GetEquipment();
		if (!!equipment.Get())
		{
			equipment->Equip();
			ChangeType(InType);
		}
	}
}

void UCWeaponComponent::ChangeType(EWeaponType InType)
{
	PrevType = Type;
	Type = InType;

	if (OnWeaponTypeChanged.IsBound())
		OnWeaponTypeChanged.Broadcast(PrevType, InType);
}

ACAttachment* UCWeaponComponent::GetAttachment() const
{
	CheckTrueResult(IsUnarmedMode(), nullptr);
	CheckFalseResult(!!DataAssets[static_cast<uint8>(Type)], nullptr);

	return DataAssetsCopy[static_cast<uint8>(Type)]->GetAttachment();
}

UCEquipment* UCWeaponComponent::GetEquipment() const
{
	CheckTrueResult(IsUnarmedMode(), nullptr);
	CheckFalseResult(!!DataAssets[static_cast<uint8>(Type)], nullptr);

	return DataAssetsCopy[static_cast<uint8>(Type)]->GetEquipment();
}

UCAct* UCWeaponComponent::GetAct() const
{
	CheckTrueResult(IsUnarmedMode(), nullptr);
	CheckFalseResult(!!DataAssets[static_cast<uint8>(Type)], nullptr);

	return DataAssetsCopy[static_cast<uint8>(Type)]->GetAct();
}

bool UCWeaponComponent::IsIdleStateMode() const
{
	return Owner->StateComp->IsIdleMode();
}

void UCWeaponComponent::SetUnarmedMode()
{
	CheckNull(GetEquipment());
	GetEquipment()->Unequip();
	ChangeType(EWeaponType::Unarmed);
}

void UCWeaponComponent::SetFistMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(EWeaponType::Fist);
}

void UCWeaponComponent::SetSwordMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(EWeaponType::Sword);
}

void UCWeaponComponent::SetHammerMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(EWeaponType::Hammer);
}

void UCWeaponComponent::SetBowMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(EWeaponType::Bow);
}

void UCWeaponComponent::SetDualMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(EWeaponType::Dual);
}

void UCWeaponComponent::SetWarpMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(EWeaponType::Warp);
}

void UCWeaponComponent::SetAroundMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(EWeaponType::Around);
}

void UCWeaponComponent::SetFireballMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(EWeaponType::Fireball);
}

void UCWeaponComponent::SetBombMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(EWeaponType::Bomb);
}

void UCWeaponComponent::SetYonduMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(EWeaponType::Yondu);
}