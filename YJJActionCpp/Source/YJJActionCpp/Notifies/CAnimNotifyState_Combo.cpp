#include "Notifies/CAnimNotifyState_Combo.h"
#include "Global.h"
#include "Components/CWeaponComponent.h"
#include "Weapons/Acts/CAct_Combo.h"

FString UCAnimNotifyState_Combo::GetNotifyName_Implementation() const
{
	return "Combo";
}

void UCAnimNotifyState_Combo::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	const TWeakObjectPtr<UCWeaponComponent> weapon = CHelpers::GetComponent<UCWeaponComponent>(MeshComp->GetOwner());
	CheckNull(weapon.Get());
	CheckNull(weapon->GetAct());

	const TWeakObjectPtr<UCAct_Combo> combo = Cast<UCAct_Combo>(weapon->GetAct());
	CheckNull(combo.Get());

	combo->EnableCombo();
}

void UCAnimNotifyState_Combo::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	const TWeakObjectPtr<UCWeaponComponent> weapon = CHelpers::GetComponent<UCWeaponComponent>(MeshComp->GetOwner());
	CheckNull(weapon.Get());
	CheckNull(weapon->GetAct());

	const TWeakObjectPtr<UCAct_Combo> combo = Cast<UCAct_Combo>(weapon->GetAct());
	CheckNull(combo.Get());

	combo->DisableCombo();
}
