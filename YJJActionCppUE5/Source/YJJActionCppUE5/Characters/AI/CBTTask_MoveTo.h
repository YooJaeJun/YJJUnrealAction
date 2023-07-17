#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "CBTTask_MoveTo.generated.h"

UCLASS()
class YJJACTIONCPPUE5_API UCBTTask_MoveTo : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UCBTTask_MoveTo();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
