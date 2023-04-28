#include "CAnimNotify_End_State.h"
#include "Global.h"
#include "Character/ICharacterAnim.h"

FString UCAnimNotify_End_State::GetNotifyName_Implementation() const
{
	return "EndState";
}

void UCAnimNotify_End_State::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	IICharacterAnim* character = Cast<IICharacterAnim>(MeshComp->GetOwner());
	CheckNull(character);

	switch(StateType)
	{
	case EStateType::BackStep:
		character->End_BackStep();
		break;
	}
}