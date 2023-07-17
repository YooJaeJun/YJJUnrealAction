#include "Characters/AI/CBTTask_Rotate.h"
#include "Global.h"
#include "AIController.h"
#include "Components/CStateComponent.h"

UCBTTask_Rotate::UCBTTask_Rotate()
{
	NodeName = TEXT("CRotate");
}

EBTNodeResult::Type UCBTTask_Rotate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const TWeakObjectPtr<APawn> owner = OwnerComp.GetAIOwner()->GetPawn();
	CheckNullResult(owner, EBTNodeResult::Failed);

	const TWeakObjectPtr<UCStateComponent> stateComp =
		YJJHelpers::GetComponent<UCStateComponent>(owner.Get());
	CheckNullResult(stateComp, EBTNodeResult::Failed);

	CheckFalseResult(stateComp->GetCurHitMode() == CEHitType::None,
		EBTNodeResult::Failed);

	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
