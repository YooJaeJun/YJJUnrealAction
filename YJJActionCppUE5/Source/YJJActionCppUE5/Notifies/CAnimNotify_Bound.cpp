#include "Notifies/CAnimNotify_Bound.h"
#include "Characters/CCommonCharacter.h"
#include "Components/SkeletalMeshComponent.h"

namespace
{
	void DispatchBoundNotify(USkeletalMeshComponent* MeshComp, const bool bBeginBound)
	{
		if (false == IsValid(MeshComp))
		{
			return;
		}

		ACCommonCharacter* character = Cast<ACCommonCharacter>(MeshComp->GetOwner());
		if (false == IsValid(character))
		{
			return;
		}

		if (bBeginBound)
		{
			character->Begin_Bound();
		}
		else
		{
			character->End_Bound();
		}
	}
}

FString UCAnimNotify_Begin_Bound::GetNotifyName_Implementation() const
{
	return TEXT("Begin_Bound");
}

void UCAnimNotify_Begin_Bound::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	DispatchBoundNotify(MeshComp, true);
}

FString UCAnimNotify_End_Bound::GetNotifyName_Implementation() const
{
	return TEXT("End_Bound");
}

void UCAnimNotify_End_Bound::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	DispatchBoundNotify(MeshComp, false);
}
