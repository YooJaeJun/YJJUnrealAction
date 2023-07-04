#include "Characters/AI/CBTTask_Act.h"
#include "Global.h"
#include "Characters/AI/CAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CWeaponComponent.h"

UCBTTask_Act::UCBTTask_Act()
{
	NodeName = TEXT("CAct");
}

EBTNodeResult::Type UCBTTask_Act::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	const TWeakObjectPtr<ACCommonCharacter> owner =
		Cast<ACCommonCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	CheckNullResult(owner, EBTNodeResult::Failed);

	const TWeakObjectPtr<UCWeaponComponent> weaponComp = 
		YJJHelpers::GetComponent<UCWeaponComponent>(owner.Get());
	CheckNullResult(weaponComp, EBTNodeResult::Failed);


	weaponComp->InputAction_Act();

	return EBTNodeResult::Succeeded;
}