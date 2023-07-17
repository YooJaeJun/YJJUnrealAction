#include "Notifies/CAnimNotify_End_Hit.h"
#include "Global.h"
#include "Interfaces/CInterface_CharacterState.h"

FString UCAnimNotify_End_Hit::GetNotifyName_Implementation() const
{
	return "End_Hit";
}

void UCAnimNotify_End_Hit::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	ICInterface_CharacterState* characterState = Cast<ICInterface_CharacterState>(MeshComp->GetOwner());
	CheckNull(characterState);

	characterState->End_Hit();
}