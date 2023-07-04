#include "Characters/AI/CBTDecorator_IsInAttackRange.h"
#include "Global.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/AI/CAIController.h"
#include "Components/CCharacterStatComponent.h"

UCBTDecorator_IsInAttackRange::UCBTDecorator_IsInAttackRange()
{
	NodeName = TEXT("CanAttack");
}

bool UCBTDecorator_IsInAttackRange::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	const TWeakObjectPtr<ACCommonCharacter> owner = 
		Cast<ACCommonCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	CheckNullResult(owner, false);

	const TWeakObjectPtr<ACCommonCharacter> target = 
		Cast<ACCommonCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ACAIController::Target));
	CheckNullResult(target, false);

	const TWeakObjectPtr<UCCharacterStatComponent> statComp =
		YJJHelpers::GetComponent<UCCharacterStatComponent>(owner.Get());
	CheckNullResult(statComp, false);

	const float attackRange = statComp->GetAttackRange();
	const float distance = target->GetDistanceTo(owner.Get());

	return distance <= attackRange;
}