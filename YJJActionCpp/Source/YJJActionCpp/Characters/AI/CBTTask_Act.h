#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Components/CWeaponComponent.h"
#include "CBTTask_Act.generated.h"

UCLASS()
class YJJACTIONCPP_API UCBTTask_Act : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_Act();

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere)
		EWeaponType WeaponType = EWeaponType::Sword;
};
