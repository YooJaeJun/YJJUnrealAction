#pragma once
#include "CoreMinimal.h"
#include "Characters/AI/CBTTask_Custom.h"
#include "Components/CWeaponComponent.h"
#include "CBTTask_Act.generated.h"

UCLASS()
class YJJACTIONCPPUE5_API UCBTTask_Act : public UCBTTask_Custom
{
	GENERATED_BODY()

public:
	UCBTTask_Act();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
