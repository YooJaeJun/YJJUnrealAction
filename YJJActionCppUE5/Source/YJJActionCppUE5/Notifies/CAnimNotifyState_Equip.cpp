#include "Notifies/CAnimNotifyState_Equip.h"
#include "Global.h"
#include "Components/CWeaponComponent.h"
#include "Components/CMagicComponent.h"
#include "Weapons/CEquipment.h"

FString UCAnimNotifyState_Equip::GetNotifyName_Implementation() const
{
	return TEXT("Equip");
}

void UCAnimNotifyState_Equip::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (false == IsValid(MeshComp))
		return;

	AActor* meshOwnerActor = MeshComp->GetOwner();
	if (false == IsValid(meshOwnerActor))
		return;

	// 레거시 ANS_Equip 의 Execution Sequence: Magic Begin_Equip 과 Weapon 의 Equipment Begin_Equip 가 서로 독립 — 한쪽 없어도 다른 쪽 수행한다.
	const TObjectPtr<UCMagicComponent> magicComponentLocal =
		YJJHelpers::GetComponent<UCMagicComponent>(meshOwnerActor);
	if (IsValid(magicComponentLocal))
		magicComponentLocal->Begin_Equip();

	const TObjectPtr<UCWeaponComponent> weaponLocal =
		YJJHelpers::GetComponent<UCWeaponComponent>(meshOwnerActor);
	if (IsValid(weaponLocal))
	{
		UCEquipment* equipmentLocal = weaponLocal->GetEquipment();
		if (IsValid(equipmentLocal))
			equipmentLocal->Begin_Equip();
	}
}

void UCAnimNotifyState_Equip::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (false == IsValid(MeshComp))
		return;

	AActor* meshOwnerActor = MeshComp->GetOwner();
	if (false == IsValid(meshOwnerActor))
		return;

	const TObjectPtr<UCMagicComponent> magicComponentLocalEnd =
		YJJHelpers::GetComponent<UCMagicComponent>(meshOwnerActor);
	if (IsValid(magicComponentLocalEnd))
		magicComponentLocalEnd->End_Equip();

	const TObjectPtr<UCWeaponComponent> weaponLocalEnd =
		YJJHelpers::GetComponent<UCWeaponComponent>(meshOwnerActor);
	if (IsValid(weaponLocalEnd))
	{
		UCEquipment* equipmentLocal = weaponLocalEnd->GetEquipment();
		if (IsValid(equipmentLocal))
			equipmentLocal->End_Equip();
	}
}
