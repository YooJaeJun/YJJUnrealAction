#pragma once

#include "CoreMinimal.h"
#include "Characters/Animals/CAnimalWeapon.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "CBTTask_DragonBoss_SkillBranch.generated.h"

class UBehaviorTree;

/**
 * 레거시 /Game/Character/Animals/Dragon/AI/Task/BTTask_DragonBoss_SkillBranch 재구현.
 * 깨져 있던 BP Dragon AI 캐스트 대신 ACDragon·ACDragonWeapon 을 사용한다.
 *
 * Execute: 이동 정지 후 블랙보드 스킬(enum 바이트)로 InvokeDragonWeaponSkill,
 *          Skill Wait Time 블랙보드(float)에는 DragonWeapon::SkillWaitTime[스킬 인덱스]를 넣은 뒤 InProgress 로 잠복.
 * Tick: 레거시와 동등하게 두 무기( DragonWeapon · AnimalWeapon ) 어느 쪽도 InAction 이 아니고
 *       UCStateComponent::IsIdle 이면 성공으로 종료한다.
 */
UCLASS(DisplayName = "Dragon Boss Skill Branch")
class YJJACTIONCPPUE5_API UCBTTask_DragonBoss_SkillBranch : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_DragonBoss_SkillBranch();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (DisplayName = "Skill Type Key"))
	FBlackboardKeySelector SkillTypeKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (DisplayName = "Skill Wait Time"))
	FBlackboardKeySelector SkillWaitTimeKey;

	/** 무기 또는 SkillWaitTime 배열이 준비되지 않았을 때 BB 대기 시간에 넣을 값. */
	UPROPERTY(EditAnywhere, Category = "DragonBoss")
	float FallbackSkillWaitSeconds = 5.0f;

private:
	/** 무기 상태를 레거시 Animal Action Abort 와 같은 방식으로 한 번 재동기화한다. */
	static void DragonBoss_ApplyAbortOneWeapon(ACAnimalWeapon* WeaponScratch);
};
