#pragma once

#include "CoreMinimal.h"
#include "Characters/AI/CBTTask_Custom.h"
#include "CBTTask_Act.generated.h"

/** 레거시 BTTask_Action — 무기/마법 DoAction 후 InAction·Idle 종료까지 Tick 대기. */
UCLASS(DisplayName = "BTTask Action")
class YJJACTIONCPPUE5_API UCBTTask_Act : public UCBTTask_Custom
{
	GENERATED_BODY()

public:
	UCBTTask_Act();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
