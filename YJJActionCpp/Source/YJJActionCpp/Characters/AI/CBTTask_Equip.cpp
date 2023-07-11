#include "Characters/AI/CBTTask_Equip.h"
#include "Global.h"
#include "Characters/AI/CAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/CWeaponComponent.h"

UCBTTask_Equip::UCBTTask_Equip()
{
	NodeName = TEXT("CEquip");
}

EBTNodeResult::Type UCBTTask_Equip::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const EBTNodeResult::Type resultType =
		Super::ExecuteTask(OwnerComp, NodeMemory);
	CheckTrueResult(resultType == EBTNodeResult::Type::Failed, resultType);

	const TWeakObjectPtr<UCWeaponComponent> weaponComp =
		YJJHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	CheckNullResult(weaponComp, EBTNodeResult::Failed);

	if (weaponComp->IsUnarmedMode())
		weaponComp->SetModeFromZeroIndex();

	return EBTNodeResult::Succeeded;
}