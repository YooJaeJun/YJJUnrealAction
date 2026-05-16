#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_Moveable.generated.h"

// 레거시 /Game/Character/Enemies/Tasks/BTTask_Moveable — 소유 폰이 ACCommonCharacter 이고 상태가 IsMoveable() 일 때만 Succeeded.
UCLASS(DisplayName = "Moveable")
class YJJACTIONCPPUE5_API UCBTTask_Moveable : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_Moveable();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
