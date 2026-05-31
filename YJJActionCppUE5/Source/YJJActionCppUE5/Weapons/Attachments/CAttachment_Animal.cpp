#include "Weapons/Attachments/CAttachment_Animal.h"

#include "Characters/CCommonCharacter.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "Utilities/CLog.h"

ACAttachment_Animal::ACAttachment_Animal()
{
}

void ACAttachment_Animal::BeginPlay()
{
	Super::BeginPlay();

	Animal_AttachDirectChildrenToCollisionsReverse();
}

void ACAttachment_Animal::Animal_AttachDirectChildrenToCollisionsReverse()
{
	if (false == IsValid(Root))
	{
		CLog::Log(FString::Printf(
			TEXT("[AttachmentAnimal] Root 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	if (false == Owner.IsValid())
	{
		CLog::Log(FString::Printf(
			TEXT("[AttachmentAnimal] Owner 미설정 — %s (스폰 Owner=캐릭터 필요)"),
			*GetNameSafe(this)));
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(Owner.Get());
	if (false == IsValid(OwnerCharacter) || false == IsValid(OwnerCharacter->GetMesh()))
	{
		CLog::Log(FString::Printf(
			TEXT("[AttachmentAnimal] Owner Mesh 없음 — %s Owner=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Owner.Get())));
		return;
	}

	TArray<USceneComponent*> DirectChildren;
	Root->GetChildrenComponents(false, DirectChildren);

	// 레거시 BP: ReverseForEachLoop — 마지막 자식부터 AttachToCollision.
	for (int32 ChildIndex = DirectChildren.Num() - 1; ChildIndex >= 0; --ChildIndex)
	{
		USceneComponent* ChildComponent = DirectChildren[ChildIndex];
		if (false == IsValid(ChildComponent))
		{
			continue;
		}

		const FName CollisionName(*ChildComponent->GetName());
		AttachToCollision(CollisionName);
	}
}
