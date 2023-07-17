#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "CAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;

UCLASS(Abstract)
class YJJACTIONCPPUE5_API ACAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACAIController();
	virtual void OnPossess(APawn* InPawn) override;

	void RunAI();
	void StopAI() const;

	UFUNCTION()
		void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

public:
	static const FName SelfActor;
	static const FName Target;
	static const FName Behavior;
	static const FName TargetLocation;

private:
	UPROPERTY()
		TObjectPtr<UBehaviorTree> BTAsset;

	UPROPERTY()
		TObjectPtr<UBlackboardData> BBAsset;

	UPROPERTY(EditAnywhere)
		TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(EditAnywhere)
		TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(EditAnywhere)
		float SightRadius = 1500.0f;

	UPROPERTY(EditAnywhere)
		float LoseSightRadius = 1800.0f;

	UPROPERTY(EditAnywhere)
		float Angle = 120.0f;

	UPROPERTY(EditAnywhere)
		float MaxAge = 5.0f;
};
