#include "Characters/AI/CBTTask_Custom.h"
#include "Global.h"
#include "AIController.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"

EBTNodeResult::Type UCBTTask_Custom::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Owner = Cast<ACCommonCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	CheckNullResult(Owner, EBTNodeResult::Failed);

	StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());
	CheckNullResult(StateComp, EBTNodeResult::Failed);

	MovementComp = YJJHelpers::GetComponent<UCMovementComponent>(Owner.Get());
	CheckNullResult(MovementComp, EBTNodeResult::Failed);

	CheckFalseResult(
		StateComp->GetCurHitMode() == CEHitType::None,
		EBTNodeResult::Failed);

	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
