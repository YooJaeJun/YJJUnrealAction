#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_EndAnimalAction.generated.h"

// 레거시 /Game/Notifies/AN_End_AnimalAction — BP_Animal_AI·Weapon 깨진 그래프를 ACAnimal + ACAnimalWeapon::End_DoAction 으로 대체한다.
UCLASS(meta = (DisplayName = "End Animal Action"))
class YJJACTIONCPPUE5_API UCAnimNotify_EndAnimalAction : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
