#include "Notifies/CAnimNotify_Begin_Act.h"
#include "Global.h"
#include "Components/CWeaponComponent.h"
#include "Weapons/CAct.h"

FString UCAnimNotify_Begin_Act::GetNotifyName_Implementation() const
{
	return "Begin_Act";
}

void UCAnimNotify_Begin_Act::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	const TWeakObjectPtr<UCWeaponComponent> weapon = YJJHelpers::GetComponent<UCWeaponComponent>(MeshComp->GetOwner());
	CheckNull(weapon.Get());
	CheckNull(weapon->GetAct());

	weapon->GetAct()->Begin_Act();
}
