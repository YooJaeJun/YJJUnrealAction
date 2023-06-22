#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "CAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;

UCLASS()
class YJJACTIONCPP_API ACAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACAIController();
	virtual void OnPossess(APawn* InPawn) override;

	void RunAI();
	void StopAI() const;

public:
	static const FName SelfActor;
	static const FName Target;
	static const FName Behavior;
	static const FName TargetLocation;

private:
	UPROPERTY()
		UBehaviorTree* BTAsset;

	UPROPERTY()
		UBlackboardData* BBAsset;
};
