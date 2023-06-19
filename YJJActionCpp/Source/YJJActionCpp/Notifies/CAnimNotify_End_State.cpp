#include "CAnimNotify_End_State.h"
#include "Global.h"
#include "Interfaces/CInterface_CharacterAnim.h"

FString UCAnimNotify_End_State::GetNotifyName_Implementation() const
{
	return "End_State";
}

void UCAnimNotify_End_State::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	ICInterface_CharacterAnim* character = Cast<ICInterface_CharacterAnim>(MeshComp->GetOwner());
	CheckNull(character);

	switch(StateType)
	{
	case EStateType::Avoid:
		character->End_Avoid();
		break;
	case EStateType::Hit:
		character->End_Hit();
		break;
	case EStateType::Dead:
		character->End_Dead();
		break;
	case EStateType::Rise:
		character->End_Rise();
		break;
	}
}