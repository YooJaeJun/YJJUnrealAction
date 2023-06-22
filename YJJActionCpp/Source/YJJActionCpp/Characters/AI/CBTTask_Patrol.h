#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_Patrol.generated.h"

UCLASS()
class YJJACTIONCPP_API UCBTTask_Patrol : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_Patrol();

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, 
		uint8* NodeMemory) override;

private:
	const float PatrolDistance = 600.0f;
};
