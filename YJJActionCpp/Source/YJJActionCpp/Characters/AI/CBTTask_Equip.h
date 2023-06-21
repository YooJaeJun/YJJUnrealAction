#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Components/CWeaponComponent.h"
#include "CBTTask_Equip.generated.h"

UCLASS()
class YJJACTIONCPP_API UCBTTask_Equip : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_Equip();

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;
};
