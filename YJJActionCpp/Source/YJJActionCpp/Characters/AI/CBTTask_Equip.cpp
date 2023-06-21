#include "Characters/AI/CBTTask_Equip.h"
#include "Global.h"
#include "Characters/AI/CAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/CEnemy_AI.h"
#include "Components/CWeaponComponent.h"

UCBTTask_Equip::UCBTTask_Equip()
{
	NodeName = TEXT("CEquip");
}

EBTNodeResult::Type UCBTTask_Equip::ExecuteTask(
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
		weaponComp->SetModeFromDataTable();

	return EBTNodeResult::Succeeded;
}