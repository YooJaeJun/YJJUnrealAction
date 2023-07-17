#include "Notifies/CAnimNotify_BowString.h"
#include "Global.h"
#include "Components/CWeaponComponent.h"
#include "Weapons/Acts/CAct_Bow.h"

FString UCAnimNotify_BowString::GetNotifyName_Implementation() const
{
	return "End_BowString";
}

void UCAnimNotify_BowString::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	const TWeakObjectPtr<UCWeaponComponent> weapon = 
		YJJHelpers::GetComponent<UCWeaponComponent>(MeshComp->GetOwner());
	CheckNull(weapon);
	CheckNull(weapon->GetAct());

	const TWeakObjectPtr<UCAct_Bow> bow = Cast<UCAct_Bow>(weapon->GetAct());
	CheckNull(bow);

	bow->End_BowString();
}