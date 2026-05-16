#pragma once
#include "CoreMinimal.h"
#include "Characters/AI/CBTTask_Custom.h"
#include "Components/CMovementComponent.h"
#include "CBTTask_Speed.generated.h"

UCLASS(DisplayName="Speed")
class YJJACTIONCPPUE5_API UCBTTask_Speed : public UCBTTask_Custom
{
	GENERATED_BODY()

public:
	UCBTTask_Speed();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Task")
	CESpeedType SpeedType = CESpeedType::Sprint;
};
