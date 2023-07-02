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

	for (int32 i = 0; i < DataAssets.Num(); i++)
	{
		if (!!DataAssets[i])
		{
			// 주석처럼 쓰면 객체 공유하게 됨
			//UCWeaponAsset* asset = NewObject<UCWeaponAsset>(this, DataAssets[i]->GetClass(), static_cast<FName>(DataAssets[i]->GetClass()->GetName()));
			UCWeaponAsset* asset = NewObject<UCWeaponAsset>(this, UCWeaponAsset::StaticClass());

			DataAssetsCopied.Add(asset);

			DataAssetsCopied[i]->DeepCopy(*DataAssets[i], Owner);
		}
	}
}

void UCWeaponComponent::InputAction_Act()
{
	CheckNull(GetAct());
	GetAct()->Act();
}

void UCWeaponComponent::SetModeFromZeroIndex()
{
	CheckFalse(DataAssetsCopied.Num() > 0);
	SetMode(DataAssetsCopied[0]->GetType());
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

	const int32 index = FindType(InType);

	CheckTrue(index == CHelpers::Npos());
	CheckNull(DataAssetsCopied[index]);

	const TWeakObjectPtr<UCEquipment> equipment = DataAssetsCopied[index]->GetEquipment();
	CheckNull(equipment);

	equipment->Equip();
	ChangeType(InType);
}

int32 UCWeaponComponent::FindType(const EWeaponType InType)
{
	for (int32 i = 0; i < DataAssetsCopied.Num(); i++)
		if (DataAssetsCopied[i]->GetType() == InType)
			return i;

	return -1;
}

void UCWeaponComponent::CancelAct()
{
	CheckNull(GetAct());
	GetAct()->End_Act();
}

void UCWeaponComponent::ChangeType(EWeaponType InType)
{
	PrevType = Type;
	Type = InType;

	if (OnWeaponTypeChanged.IsBound())
		OnWeaponTypeChanged.Broadcast(PrevType, InType);
}

ACAttachment* UCWeaponComponent::GetAttachment()
{
	CheckTrueResult(IsUnarmedMode(), nullptr);

	const int32 weaponType = FindType(Type);
	CheckTrueResult(weaponType == CHelpers::Npos(), nullptr);

	return DataAssetsCopied[weaponType]->GetAttachment();
}

UCEquipment* UCWeaponComponent::GetEquipment()
{
	CheckTrueResult(IsUnarmedMode(), nullptr);

	const int32 weaponType = FindType(Type);
	CheckTrueResult(weaponType == CHelpers::Npos(), nullptr);

	return DataAssetsCopied[weaponType]->GetEquipment();
}

UCAct* UCWeaponComponent::GetAct()
{
	CheckTrueResult(IsUnarmedMode(), nullptr);

	const int32 weaponType = FindType(Type);
	CheckTrueResult(weaponType == CHelpers::Npos(), nullptr);

	return DataAssetsCopied[weaponType]->GetAct();
}

bool UCWeaponComponent::IsIdleStateMode() const
{
	CheckNullResult(Owner->StateComp, false);
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