#pragma once

#include "CoreMinimal.h"
#include "LegacyCameraShake.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_CameraShake.generated.h"

UCLASS()
class YJJACTIONCPPUE5_API UCAnimNotify_CameraShake : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere, Category = "CameraShakeClass")
		TSubclassOf<ULegacyCameraShake> CameraShakeClass;
};
