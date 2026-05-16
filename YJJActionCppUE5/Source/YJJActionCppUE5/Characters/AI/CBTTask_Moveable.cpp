#include "Characters/AI/CBTTask_Moveable.h"
#include "Global.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "GameFramework/Pawn.h"

UCBTTask_Moveable::UCBTTask_Moveable()
{
	NodeName = TEXT("Moveable");
}

EBTNodeResult::Type UCBTTask_Moveable::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* aiOwner = OwnerComp.GetAIOwner();
	if (false == IsValid(aiOwner))
		return EBTNodeResult::Failed;

	APawn* controlledPawn = aiOwner->GetPawn();
	if (false == IsValid(controlledPawn))
		return EBTNodeResult::Failed;

	ACCommonCharacter* commonCharacter = Cast<ACCommonCharacter>(controlledPawn);
	if (false == IsValid(commonCharacter))
		return EBTNodeResult::Failed;

	const TObjectPtr<UCStateComponent> stateComp =
		YJJHelpers::GetComponent<UCStateComponent>(commonCharacter);
	if (false == IsValid(stateComp))
		return EBTNodeResult::Failed;

	if (stateComp->IsMoveable())
		return EBTNodeResult::Succeeded;

	return EBTNodeResult::Failed;
}
