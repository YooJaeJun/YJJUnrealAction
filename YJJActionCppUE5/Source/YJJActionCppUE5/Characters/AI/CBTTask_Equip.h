#pragma once
#include "CoreMinimal.h"
#include "Characters/AI/CBTTask_Custom.h"
#include "Components/CWeaponComponent.h"
#include "CBTTask_Equip.generated.h"

UCLASS()
class YJJACTIONCPPUE5_API UCBTTask_Equip : public UCBTTask_Custom
{
	GENERATED_BODY()

public:
	UCBTTask_Equip();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};