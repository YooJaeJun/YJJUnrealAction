#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Components/CMovementComponent.h"
#include "CBTTask_Speed.generated.h"

UCLASS()
class YJJACTIONCPP_API UCBTTask_Speed : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_Speed();

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere)
		ESpeedType SpeedType = ESpeedType::Sprint;
};
