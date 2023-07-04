#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CMotionTrail.generated.h"

class UPoseableMeshComponent;
class ACCommonCharacter;
class UMaterialInstanceDynamic;

UCLASS()
class YJJACTIONCPP_API ACMotionTrail : public AActor
{
	GENERATED_BODY()
	
public:	
	ACMotionTrail();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Capture")
		float StartDelay = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Capture")
		float Interval = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Capture")
		FLinearColor Color = FLinearColor(1, 1, 1, 1);

	UPROPERTY(EditDefaultsOnly, Category = "Capture")
		float Exponent = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Capture")
		FVector Scale = FVector::OneVector;

	UPROPERTY(EditDefaultsOnly, Category = "Capture")
		FVector ScaleAmount = FVector::ZeroVector;

private:
	UPROPERTY(VisibleDefaultsOnly)
		UPoseableMeshComponent* Mesh;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UMaterialInstanceDynamic> Material;

	FTimerHandle TimerHandle;
};
