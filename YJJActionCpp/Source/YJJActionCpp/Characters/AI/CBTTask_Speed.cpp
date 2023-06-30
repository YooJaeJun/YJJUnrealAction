#include "Characters/AI/CBTTask_Speed.h"
#include "Global.h"
#include "AIController.h"

UCBTTask_Speed::UCBTTask_Speed()
{
	NodeName = TEXT("CSpeed");
}

EBTNodeResult::Type UCBTTask_Speed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	const TWeakObjectPtr<APawn> owner =
		OwnerComp.GetAIOwner()->GetPawn();
	CheckNullResult(owner, EBTNodeResult::Failed);

	const TWeakObjectPtr<UCMovementComponent> movement =
		CHelpers::GetComponent<UCMovementComponent>(owner.Get());
	CheckNullResult(movement, EBTNodeResult::Failed);


	movement->SetSpeed(SpeedType);

	return EBTNodeResult::Succeeded;
}
