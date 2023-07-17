#include "Characters/AI/CBTTask_Act.h"
#include "Global.h"
#include "Characters/AI/CAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/CWeaponComponent.h"

UCBTTask_Act::UCBTTask_Act()
{
	NodeName = TEXT("CAct");
}

EBTNodeResult::Type UCBTTask_Act::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const EBTNodeResult::Type resultType = Super::ExecuteTask(OwnerComp, NodeMemory);
	CheckTrueResult(resultType == EBTNodeResult::Type::Failed, resultType);

	const TWeakObjectPtr<UCWeaponComponent> weaponComp = 
		YJJHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	CheckNullResult(weaponComp, EBTNodeResult::Failed);

	weaponComp->InputAction_Act();

	return EBTNodeResult::Succeeded;
}