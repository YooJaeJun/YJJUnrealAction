#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_CameraShake.generated.h"

class UMatineeCameraShake;

UCLASS()
class YJJACTIONCPP_API UCAnimNotify_CameraShake : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	UPROPERTY(EditAnywhere, Category = "CameraShakeClass")
		TSubclassOf<UMatineeCameraShake> CameraShakeClass;
};
