#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CAnimNotifyState_AnimalCollision.generated.h"

// 레거시 /Game/Notifies/ANS_Animal_Collision — BP_Animal_AI 캐스트 → Weapon.OnCollisions/OffCollisions 를
// ACAnimal + ACAnimalWeapon(드래곤은 DragonWeapon 블프 캐스트) 경로로 네이티브화했다.
UCLASS(meta = (DisplayName = "Animal Collision"))
class YJJACTIONCPPUE5_API UCAnimNotifyState_AnimalCollision : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
