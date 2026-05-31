#include "Notifies/CAnimNotifyState_Collision.h"
#include "Components/CWeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

FString UCAnimNotifyState_Collision::GetNotifyName_Implementation() const
{
	return TEXT("Collision");
}

void UCAnimNotifyState_Collision::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

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
		// 비전투 액터 등 — Collision 노티 공용이라 로그는 남기지 않는다.
		return;
	}

	weaponComp->ApplyLegacyMainWeaponCollisionBound(true);
}

void UCAnimNotifyState_Collision::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

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

	weaponComp->ApplyLegacyMainWeaponCollisionBound(false);
}
