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

	UPROPERTY(EditDefaultsOnly, Category = "Capture")
		bool DisappearFlag = false;

	UPROPERTY(EditDefaultsOnly, Category = "Capture")
		float DisappearStartDelay = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Capture")
		float DisappearInterval = 0.01f;

	UPROPERTY(EditDefaultsOnly, Category = "Capture")
		float DisappearExponent = 0.1f;

private:
	UPROPERTY(VisibleDefaultsOnly)
		UPoseableMeshComponent* Mesh;

	UPROPERTY(VisibleDefaultsOnly)
		UMaterialInstanceDynamic* Material;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;

	FTimerHandle TimerHandle;
	FTimerHandle TimerDisappearHandle;

	float OriginalExponent = 0.0f;
};
