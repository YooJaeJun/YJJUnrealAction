#include "Notifies/CAnimNotify_End_Act.h"
#include "Global.h"
#include "Components/CWeaponComponent.h"
#include "Weapons/CAct.h"

FString UCAnimNotify_End_Act::GetNotifyName_Implementation() const
{
	return "End_Act";
}

void UCAnimNotify_End_Act::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	const TWeakObjectPtr<UCWeaponComponent> weapon = CHelpers::GetComponent<UCWeaponComponent>(MeshComp->GetOwner());
	CheckNull(weapon);
	CheckNull(weapon->GetAct());

	weapon->GetAct()->End_Act();
}
