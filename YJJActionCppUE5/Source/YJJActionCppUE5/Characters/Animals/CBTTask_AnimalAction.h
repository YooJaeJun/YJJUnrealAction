#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "CBTTask_AnimalAction.generated.h"

/**
 * 레거시 /Game/Character/Animals/BTTask_Animal_Action —
 * BP_Animal_AI 캐스트·StateComponent 노드 깨짐을 ACAnimal 경로로 복구한다.
 * 블랙보드 타깃 액터 위치로 무기 DoAction 재생 후, 무기 !InAction + UCStateComponent::IsIdle 이 될 때까지 Tick 하다 성공한다.
 */
UCLASS(DisplayName = "Animal Action")
class YJJACTIONCPPUE5_API UCBTTask_AnimalAction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_AnimalAction();

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (DisplayName = "Target"))
	FBlackboardKeySelector TargetBlackboardKey;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
