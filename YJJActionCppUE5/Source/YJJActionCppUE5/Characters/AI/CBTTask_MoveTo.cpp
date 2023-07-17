#include "Characters/AI/CBTTask_MoveTo.h"
#include "Global.h"
#include "Components/CStateComponent.h"
#include "Characters/AI/CAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UCBTTask_MoveTo::UCBTTask_MoveTo()
{
	NodeName = TEXT("CMoveTo");
}

EBTNodeResult::Type UCBTTask_MoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const TWeakObjectPtr<APawn> owner =
		OwnerComp.GetAIOwner()->GetPawn();
	CheckNullResult(owner, EBTNodeResult::Failed);

	const TWeakObjectPtr<UCStateComponent> stateComp =
		YJJHelpers::GetComponent<UCStateComponent>(owner.Get());
	CheckNullResult(stateComp, EBTNodeResult::Failed);

	CheckFalseResult(stateComp->GetCurHitMode() == CEHitType::None,
		EBTNodeResult::Failed);

	return Super::ExecuteTask(OwnerComp, NodeMemory);
}