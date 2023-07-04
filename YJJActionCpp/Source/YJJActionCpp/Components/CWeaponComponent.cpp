#include "Components/CWeaponComponent.h"
#include "Global.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CEquipment.h"
#include "Weapons/CAct.h"
#include "Weapons/CSkill.h"

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

			// 깊은 복사로 객체 공유 막고, map에 넣어 find 용이하게 관리
			UCWeaponAsset* asset = NewObject<UCWeaponAsset>(this, UCWeaponAsset::StaticClass());

			asset->DeepCopy(*DataAssets[i], Owner);

			DataAssetMap.Emplace(asset->GetType(), asset);
		}
	}
}

void UCWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const TWeakObjectPtr<UCAct> act = GetAct();
	if (!!act.Get())
		act->Tick(DeltaTime);

	const TWeakObjectPtr<UCSkill> skill = GetSkill();
	if (!!skill.Get())
		skill->Tick(DeltaTime);
}

void UCWeaponComponent::InputAction_Act()
{
	const TWeakObjectPtr<UCAct> act = GetAct();
	CheckNull(act);
	act->Act();
}

void UCWeaponComponent::InputAction_Skill_Pressed()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill();
	CheckNull(skill);
	skill->Pressed();
}

void UCWeaponComponent::InputAction_Skill_Released()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill();
	CheckNull(skill);
	skill->Released();
}

void UCWeaponComponent::SetModeFromZeroIndex()
{
	CheckFalse(DataAssetMap.Num() > 0);
	SetMode(DataAssetMap.begin().Key());
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

	const TWeakObjectPtr<UCWeaponAsset> asset = *DataAssetMap.Find(InType);
	CheckNull(asset);
	const TWeakObjectPtr<UCEquipment> equipment = asset->GetEquipment();
	CheckNull(equipment);

	equipment->Equip();
	ChangeType(InType);
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

TWeakObjectPtr<UCWeaponAsset> UCWeaponComponent::GetWeaponAsset()
{
	CheckTrueResult(IsUnarmedMode(), nullptr);

	const TWeakObjectPtr<UCWeaponAsset>* weaponAsset = DataAssetMap.Find(Type);
	CheckNullResult(*weaponAsset, nullptr);

	const TWeakObjectPtr<UCWeaponAsset> ret = *weaponAsset;
	return ret;
}

ACAttachment* UCWeaponComponent::GetAttachment()
{
	const TWeakObjectPtr<UCWeaponAsset> asset = GetWeaponAsset();
	CheckNullResult(asset, nullptr);

	return asset->GetAttachment();
}

UCEquipment* UCWeaponComponent::GetEquipment()
{
	const TWeakObjectPtr<UCWeaponAsset> asset = GetWeaponAsset();
	CheckNullResult(asset, nullptr);

	return asset->GetEquipment();
}

UCAct* UCWeaponComponent::GetAct()
{
	const TWeakObjectPtr<UCWeaponAsset> asset = GetWeaponAsset();
	CheckNullResult(asset, nullptr);

	return asset->GetAct();
}

UCSkill* UCWeaponComponent::GetSkill()
{
	const TWeakObjectPtr<UCWeaponAsset> asset = GetWeaponAsset();
	CheckNullResult(asset, nullptr);

	return asset->GetSkill();
}

bool UCWeaponComponent::IsIdleStateMode()
{
	StateComp = CHelpers::GetComponent<UCStateComponent>(Owner.Get());
	CheckNullResult(StateComp, false);
	return StateComp->IsIdleMode();
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