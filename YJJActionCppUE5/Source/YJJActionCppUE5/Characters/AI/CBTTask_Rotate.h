#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_RotateToFaceBBEntry.h"
#include "CBTTask_Rotate.generated.h"

UCLASS()
class YJJACTIONCPPUE5_API UCBTTask_Rotate : public UBTTask_RotateToFaceBBEntry
{
	GENERATED_BODY()

public:
	UCBTTask_Rotate();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
