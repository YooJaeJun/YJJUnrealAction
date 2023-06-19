#pragma once
#include "CoreMinimal.h"
#include "Characters/AI/CAIController.h"
#include "CAIController_Melee.generated.h"

class UBehaviorTree;
class UBlackboardData;

UCLASS()
class YJJACTIONCPP_API ACAIController_Melee : public ACAIController
{
	GENERATED_BODY()

public:
	ACAIController_Melee();
	virtual void OnPossess(APawn* InPawn) override;

	void RunAI();
	void StopAI() const;

private:
	static const FName Target;
	static const FName Behavior;
	static const FName EQLocation;

private:
	UPROPERTY()
		UBehaviorTree* BTAsset;

	UPROPERTY()
		UBlackboardData* BBAsset;
};
