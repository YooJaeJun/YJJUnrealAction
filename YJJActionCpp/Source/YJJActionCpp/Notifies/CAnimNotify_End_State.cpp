#include "Notifies/CAnimNotify_End_State.h"
#include "Global.h"
#include "Interfaces/CInterface_CharacterState.h"

FString UCAnimNotify_End_State::GetNotifyName_Implementation() const
{
	return "End_State";
}

void UCAnimNotify_End_State::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	ICInterface_CharacterState* characterState = Cast<ICInterface_CharacterState>(MeshComp->GetOwner());
	CheckNull(characterState);

	switch(StateType)
	{
	case EStateType::Avoid:
		characterState->End_Avoid();
		break;
	case EStateType::Dead:
		characterState->End_Dead();
		break;
	case EStateType::Rise:
		characterState->End_Rise();
		break;
	}
}