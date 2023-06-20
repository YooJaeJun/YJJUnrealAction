#include "Characters/AI/CBTTask_Act.h"
#include "Global.h"
#include "Characters/AI/CAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/CEnemy_AI.h"
#include "Components/CWeaponComponent.h"

UCBTTask_Act::UCBTTask_Act()
{
	NodeName = TEXT("CAct");
}

EBTNodeResult::Type UCBTTask_Act::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	const TWeakObjectPtr<ACEnemy_AI> owner = 
		Cast<ACEnemy_AI>(OwnerComp.GetAIOwner()->GetPawn());
	CheckNullResult(owner, EBTNodeResult::Failed);

	const TWeakObjectPtr<UCWeaponComponent> weaponComp = 
		CHelpers::GetComponent<UCWeaponComponent>(owner.Get());
	CheckNullResult(weaponComp, EBTNodeResult::Failed);

	if (weaponComp->IsUnarmedMode())
		weaponComp->SetMode(WeaponType);

	weaponComp->InputAction_Act();

	return EBTNodeResult::Succeeded;
}