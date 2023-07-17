#include "Notifies/CAnimNotify_End_State.h"
#include "Global.h"
#include "Interfaces/CInterface_CharacterState.h"

FString UCAnimNotify_End_State::GetNotifyName_Implementation() const
{
	return "End_State";
}

void UCAnimNotify_End_State::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	ICInterface_CharacterState* characterState = Cast<ICInterface_CharacterState>(MeshComp->GetOwner());
	CheckNull(characterState);

	switch(StateType)
	{
	case CEStateType::Avoid:
		characterState->End_Avoid();
		break;
	case CEStateType::Dead:
		characterState->End_Dead();
		break;
	case CEStateType::Rise:
		characterState->End_Rise();
		break;
	}
}