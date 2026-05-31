#include "Notifies/CAnimNotifyHelpers.h"
#include "Components/CWeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UCWeaponComponent* CAnimNotifyHelpers_ResolveWeaponComponent(USkeletalMeshComponent* MeshComp)
{
	if (false == IsValid(MeshComp))
	{
		return nullptr;
	}

	AActor* meshOwnerActor = MeshComp->GetOwner();
	if (false == IsValid(meshOwnerActor))
	{
		return nullptr;
	}

	return meshOwnerActor->FindComponentByClass<UCWeaponComponent>();
}
