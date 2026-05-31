#include "Notifies/CAnimNotify_Begin_Action.h"
#include "Commons/CEnums.h"
#include "Components/CWeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"

FString UCAnimNotify_Begin_Action::GetNotifyName_Implementation() const
{
	return TEXT("Begin_Action");
}

void UCAnimNotify_Begin_Action::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (false == IsValid(MeshComp))
	{
		return;
	}

	AActor* meshOwnerActor = MeshComp->GetOwner();
	if (false == IsValid(meshOwnerActor))
	{
		return;
	}

	UCWeaponComponent* weaponComp = meshOwnerActor->FindComponentByClass<UCWeaponComponent>();
	if (false == IsValid(weaponComp))
	{
		return;
	}

	weaponComp->Begin_DoAction(AttackType);
}
