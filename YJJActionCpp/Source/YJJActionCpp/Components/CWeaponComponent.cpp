#include "Components/CWeaponComponent.h"
#include "Global.h"
#include "Components/CStateComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CEquipment.h"
#include "Weapons/CAct.h"
#include "Weapons/CWeaponAsset.h"

UCWeaponComponent::UCWeaponComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	CheckNull(Owner);

	uint8 size = (uint8)EWeaponType::Max;
	for (int32 i = 0; i < size; i++)
	{
		if (!!DataAssets[i])
			DataAssets[i]->BeginPlay(Owner.Get());
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
		DataAssets[static_cast<uint8>(InType)]->GetEquipment()->Equip();
		ChangeType(InType);
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

	return DataAssets[static_cast<uint8>(Type)]->GetAttachment();
}

UCEquipment* UCWeaponComponent::GetEquipment() const
{
	CheckTrueResult(IsUnarmedMode(), nullptr);
	CheckFalseResult(!!DataAssets[static_cast<uint8>(Type)], nullptr);

	return DataAssets[static_cast<uint8>(Type)]->GetEquipment();
}

UCAct* UCWeaponComponent::GetAct() const
{
	CheckTrueResult(IsUnarmedMode(), nullptr);
	CheckFalseResult(!!DataAssets[static_cast<uint8>(Type)], nullptr);

	return DataAssets[static_cast<uint8>(Type)]->GetAct();
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