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

	ICInterface_CharacterAnim* characterAnim = Cast<ICInterface_CharacterAnim>(MeshComp->GetOwner());
	CheckNull(characterAnim);

	switch(StateType)
	{
	case EStateType::Avoid:
		characterAnim->End_Avoid();
		break;
	case EStateType::Hit:
		characterAnim->End_Hit();
		break;
	case EStateType::Dead:
		characterAnim->End_Dead();
		break;
	case EStateType::Rise:
		characterAnim->End_Rise();
		break;
	}
}