#include "Characters/AI/CBTDecorator_IsInAttackRange.h"
#include "Global.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/AI/CAIController.h"

UCBTDecorator_IsInAttackRange::UCBTDecorator_IsInAttackRange()
{
	NodeName = TEXT("CanAttack");
}

bool UCBTDecorator_IsInAttackRange::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	const TWeakObjectPtr<APawn> owner = OwnerComp.GetAIOwner()->GetPawn();
	CheckNullResult(owner, false);

	const TWeakObjectPtr<ACCommonCharacter> target = Cast<ACCommonCharacter>(
		OwnerComp.GetBlackboardComponent()->GetValueAsObject(ACAIController::Target));
	CheckNullResult(target, false);

	const bool bResult = (target->GetDistanceTo(owner.Get()) <= 200.0f);
	return bResult;
}