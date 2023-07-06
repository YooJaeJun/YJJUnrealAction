#include "Notifies/CAnimNotify_End_Hit.h"
#include "Global.h"
#include "Interfaces/CInterface_CharacterState.h"

FString UCAnimNotify_End_Hit::GetNotifyName_Implementation() const
{
	return "End_Hit";
}

void UCAnimNotify_End_Hit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	ICInterface_CharacterState* characterState = Cast<ICInterface_CharacterState>(MeshComp->GetOwner());
	CheckNull(characterState);

	switch (HitType)
	{
	case EHitType::Common:
		characterState->End_Hit();
		break;
	case EStateType::Dead:
		characterState->End_Dead();
		break;
	case EStateType::Rise:
		characterState->End_Rise();
		break;
	}
}