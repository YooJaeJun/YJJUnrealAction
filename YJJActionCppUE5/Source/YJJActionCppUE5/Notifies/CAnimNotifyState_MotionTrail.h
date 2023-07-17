#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Weapons/AddOns/CMotionTrail.h"
#include "CAnimNotifyState_MotionTrail.generated.h"

UCLASS()
class YJJACTIONCPPUE5_API UCAnimNotifyState_MotionTrail : public UAnimNotifyState
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

private:
	UPROPERTY(EditAnywhere, Category = "Add-On")
		TSubclassOf<ACMotionTrail> MotionTrailClass;

private:
	TWeakObjectPtr<ACMotionTrail> MotionTrail;
};
