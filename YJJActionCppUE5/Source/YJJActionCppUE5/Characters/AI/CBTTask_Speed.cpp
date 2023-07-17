#include "Characters/AI/CBTTask_Speed.h"
#include "Global.h"
#include "AIController.h"

UCBTTask_Speed::UCBTTask_Speed()
{
	NodeName = TEXT("CSpeed");
}

EBTNodeResult::Type UCBTTask_Speed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const EBTNodeResult::Type resultType =
		Super::ExecuteTask(OwnerComp, NodeMemory);
	CheckTrueResult(resultType == EBTNodeResult::Type::Failed, resultType);

	MovementComp->SetSpeed(SpeedType);


	return EBTNodeResult::Succeeded;
}