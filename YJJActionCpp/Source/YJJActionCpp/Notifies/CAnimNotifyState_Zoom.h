#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CAnimNotifyState_Zoom.generated.h"

UCLASS()
class YJJACTIONCPP_API UCAnimNotifyState_Zoom : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	UPROPERTY(EditAnywhere, Category = "Zoom")
		float OriginZooming;

	UPROPERTY(EditAnywhere, Category = "Zoom")
		float SkillZooming = 250.0f;
};
