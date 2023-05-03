#include "Component/CWeaponComponent.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "Weapons/CWeapon.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CEquipment.h"
#include "Weapons/CWeaponAsset.h"

UCWeaponComponent::UCWeaponComponent()
{
}

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ACCommonCharacter>(GetOwner());
	CheckNull(Owner);

	FActorSpawnParameters params;
	params.Owner = Owner.Get();
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	uint8 size = (uint8)EWeaponType::Max;
	for (int32 i=0; i<size; i++)
	{
		if (!!DataAssets[i])
			DataAssets[i]->BeginPlay(Owner.Get());
	}
}

ACAttachment* UCWeaponComponent::GetAttachment()
{
	CheckTrueResult(IsUnarmedMode(), nullptr);
	CheckFalseResult(!!DataAssets[static_cast<uint8>(Type)], nullptr);

	return DataAssets[static_cast<uint8>(Type)]->GetAttachment();
}

UCEquipment* UCWeaponComponent::GetEquipment()
{
	CheckTrueResult(IsUnarmedMode(), nullptr);
	CheckFalseResult(!!DataAssets[static_cast<uint8>(Type)], nullptr);

	return DataAssets[static_cast<uint8>(Type)]->GetEquipment();
}

bool UCWeaponComponent::IsIdleMode() const
{
	return Owner->StateComponent->IsIdleMode();
}

void UCWeaponComponent::SetUnarmedMode()
{
	CheckNull(GetEquipment());

	GetEquipment()->Unequip();

	ChangeType(EWeaponType::Unarmed);
}

void UCWeaponComponent::SetSwordMode()
{
	CheckFalse(IsIdleMode());

	SetMode(EWeaponType::Sword);
}

void UCWeaponComponent::SetFistMode()
{
	CheckFalse(IsIdleMode());

	SetMode(EWeaponType::Fist);
}

void UCWeaponComponent::SetHammerMode()
{
	CheckFalse(IsIdleMode());

	SetMode(EWeaponType::Hammer);
}

void UCWeaponComponent::SetBowMode()
{
	CheckFalse(IsIdleMode());

	SetMode(EWeaponType::Bow);
}

void UCWeaponComponent::SetDualMode()
{
	CheckFalse(IsIdleMode());

	SetMode(EWeaponType::Dual);
}

void UCWeaponComponent::SetMode(EWeaponType InType)
{
	if (Type == InType)
	{
		SetUnarmedMode();

		return;
	}

	if(false == IsUnarmedMode())
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


void UCWeaponComponent::Begin_Equip()
{
}

void UCWeaponComponent::End_Equip()
{
}

void UCWeaponComponent::Begin_Act()
{
}

void UCWeaponComponent::End_Act()
{
}