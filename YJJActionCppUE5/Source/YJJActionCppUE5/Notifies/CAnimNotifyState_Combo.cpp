#include "Notifies/CAnimNotifyState_Combo.h"
#include "Global.h"
#include "Components/CWeaponComponent.h"
#include "Weapons/Acts/CAct_Combo.h"

FString UCAnimNotifyState_Combo::GetNotifyName_Implementation() const
{
	return "Combo";
}

void UCAnimNotifyState_Combo::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	const TWeakObjectPtr<UCWeaponComponent> weapon = YJJHelpers::GetComponent<UCWeaponComponent>(MeshComp->GetOwner());
	CheckNull(weapon.Get());

	const TWeakObjectPtr<UCAct_Combo> combo = Cast<UCAct_Combo>(weapon->GetAct());
	if (combo.IsValid())
	{
		combo->EnableCombo();
		return;
	}

	// 레거시 ANS_Combo — MainWeapon(Weapon_Combo) 의 EnableCombo 직접 호출 경로.
	weapon->ApplyLegacyMainWeaponComboWindow(true);
}

void UCAnimNotifyState_Combo::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	const TWeakObjectPtr<UCWeaponComponent> weapon = YJJHelpers::GetComponent<UCWeaponComponent>(MeshComp->GetOwner());
	CheckNull(weapon.Get());

	const TWeakObjectPtr<UCAct_Combo> combo = Cast<UCAct_Combo>(weapon->GetAct());
	if (combo.IsValid())
	{
		combo->DisableCombo();
		return;
	}

	weapon->ApplyLegacyMainWeaponComboWindow(false);
}
