#include "Characters/AI/CBTTask_Patrol.h"
#include "Global.h"
#include "Characters/AI/CAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Characters/Enemies/CEnemy_AI.h"

UCBTTask_Patrol::UCBTTask_Patrol()
{
	NodeName = TEXT("CPatrol");
}

EBTNodeResult::Type UCBTTask_Patrol::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, 
	uint8* NodeMemory)
{
	const TWeakObjectPtr<APawn> owner = 
		OwnerComp.GetAIOwner()->GetPawn();
	CheckNullResult(owner, EBTNodeResult::Failed);

	const TWeakObjectPtr<UNavigationSystemV1> navSystem =
		UNavigationSystemV1::GetNavigationSystem(owner->GetWorld());
	CheckNullResult(navSystem, EBTNodeResult::Failed);


	const FVector currentLocation = owner->GetActorLocation();

	FNavLocation nextLocation;

	if (navSystem->GetRandomPointInNavigableRadius(currentLocation, PatrolDistance, nextLocation))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(ACAIController::TargetLocation, nextLocation);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}