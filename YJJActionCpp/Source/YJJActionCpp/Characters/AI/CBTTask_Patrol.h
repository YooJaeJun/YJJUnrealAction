#pragma once
#include "CoreMinimal.h"
#include "Characters/AI/CBTTask_Custom.h"
#include "CBTTask_Patrol.generated.h"

UCLASS()
class YJJACTIONCPP_API UCBTTask_Patrol : public UCBTTask_Custom
{
	GENERATED_BODY()

public:
	UCBTTask_Patrol();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere)
		float PatrolDistance = 1200.0f;
};
