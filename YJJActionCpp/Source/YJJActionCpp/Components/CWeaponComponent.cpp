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
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	CheckNull(Owner);

	for (int32 i = 0; i < DataAssets.Num(); i++)
	{
		if (IsValid(DataAssets[i]))
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

void UCWeaponComponent::TickComponent(
	float DeltaTime, 
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const TWeakObjectPtr<UCAct> act = GetAct();
	if (act.IsValid())
		act->Tick(DeltaTime);

	int index = 0;
	TWeakObjectPtr<UCSkill> skill = GetSkill(index);

	while (skill.IsValid())
	{
		skill->Tick(DeltaTime);

		index++;
		skill = GetSkill(index);
	}
}

void UCWeaponComponent::InputAction_Act()
{
	const TWeakObjectPtr<UCAct> act = GetAct();
	CheckNull(act);
	act->Act();
}

void UCWeaponComponent::InputAction_Skill_1_Pressed()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(0);
	CheckNull(skill);
	skill->Pressed();
}

void UCWeaponComponent::InputAction_Skill_1_Released()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(0);
	CheckNull(skill);
	skill->Released();
}

void UCWeaponComponent::InputAction_Skill_2_Pressed()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(1);
	CheckNull(skill);
	skill->Pressed();
}

void UCWeaponComponent::InputAction_Skill_2_Released()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(1);
	CheckNull(skill);
	skill->Released();
}

void UCWeaponComponent::InputAction_Skill_3_Pressed()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(2);
	CheckNull(skill);
	skill->Pressed();
}

void UCWeaponComponent::InputAction_Skill_3_Released()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(2);
	CheckNull(skill);
	skill->Released();
}

void UCWeaponComponent::SetModeFromZeroIndex()
{
	CheckFalse(DataAssetMap.Num() > 0);
	SetMode(DataAssetMap.begin().Key());
}

void UCWeaponComponent::SetMode(CEWeaponType InType)
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

	UCWeaponAsset** assetForCheckingNull = DataAssetMap.Find(InType);
	CheckNull(assetForCheckingNull);

	const TWeakObjectPtr<UCWeaponAsset> asset = *assetForCheckingNull;
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

void UCWeaponComponent::ChangeType(CEWeaponType InType)
{
	PrevType = Type;
	Type = InType;

	if (OnWeaponTypeChanged.IsBound())
		OnWeaponTypeChanged.Broadcast(PrevType, InType);
}

bool UCWeaponComponent::IsIdleStateMode()
{
	StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());
	CheckNullResult(StateComp, false);
	return StateComp->IsIdleMode();
}

void UCWeaponComponent::SetUnarmedMode()
{
	CheckNull(GetEquipment());
	GetEquipment()->Unequip();
	ChangeType(CEWeaponType::Unarmed);
}

void UCWeaponComponent::SetFistMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Fist);
}

void UCWeaponComponent::SetSwordMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Sword);
}

void UCWeaponComponent::SetHammerMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Hammer);
}

void UCWeaponComponent::SetBowMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Bow);
}

void UCWeaponComponent::SetDualMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Dual);
}

void UCWeaponComponent::SetWarpMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Warp);
}

void UCWeaponComponent::SetAroundMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Around);
}

void UCWeaponComponent::SetFireballMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Fireball);
}

void UCWeaponComponent::SetBombMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Bomb);
}

void UCWeaponComponent::SetYonduMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Yondu);
}

TWeakObjectPtr<UCWeaponAsset> UCWeaponComponent::GetWeaponAsset()
{
	CheckTrueResult(IsUnarmedMode(), nullptr);

	UCWeaponAsset** assetForCheckingNull = DataAssetMap.Find(Type);
	CheckNullResult(*assetForCheckingNull, nullptr);

	const TWeakObjectPtr<UCWeaponAsset> assetForReturn = *assetForCheckingNull;
	return assetForReturn;
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

UCSkill* UCWeaponComponent::GetSkill(const int32 SkillIndex)
{
	const TWeakObjectPtr<UCWeaponAsset> asset = GetWeaponAsset();
	CheckNullResult(asset, nullptr);

	return asset->GetSkill(SkillIndex);
}