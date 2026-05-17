#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Commons/CEnums.h"
#include "CBTTask_SelectSkillType.generated.h"

class UBehaviorTree;
class UBlackboardComponent;

/**
 * 레거시 /Game/Character/Animals/Dragon/AI/Task/BTTask_SelectSkillType —
 * BP_Dragon_AI 캐스트 깨짐을 ACDragon 으로 복구했다.
 *
 * 블랙보드 DistanceType 이 EDistanceType 의 Avoid(NewEnumerator18, 바이트 18 과 동명) 와 같으면
 * StopAnimMontage 후 SkillType 을 고정 바이트(블프에서는 4)로 쓰고,
 * 아니면 RandomIntegerInRange(0, 3) 과 동등한 무작위 스킬 바이트를 쓴다.
 */
UCLASS(meta = (DisplayName = "Select Skill Type"))
class YJJACTIONCPPUE5_API UCBTTask_SelectSkillType : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_SelectSkillType();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (DisplayName = "Skill Type"))
	FBlackboardKeySelector SkillTypeBlackboardKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (DisplayName = "Distance Type"))
	FBlackboardKeySelector DistanceTypeBlackboardKey;

	/** UDE 레거시 정수(`CEDistanceType::Avoid`) 와 동형 — 서비스 노드 디폴트와 동시에 조정. */
	UPROPERTY(EditAnywhere, Category = "DragonBoss|BB Bytes", meta = (ClampMin = "0"))
	uint8 DistanceAvoidByteScratch = static_cast<uint8>(CEDistanceType::Avoid);

	/** Avoid 분기 시 SkillType 블랙보드 기록값(블프 고정 바이트 4). */
	UPROPERTY(EditAnywhere, Category = "DragonBoss|BB Bytes", meta = (ClampMin = "0"))
	uint8 SkillTypeFixedByteWhenAvoid = 4;

	/** 비-Avoid 분기 시 RandomIntegerInRange 최소 포함. */
	UPROPERTY(EditAnywhere, Category = "DragonBoss|Skill Pick", meta = (ClampMin = "0"))
	int32 RandomSkillMinInclusiveScratch = 0;

	/** 비-Avoid 분기 시 RandomIntegerInRange 최대 포함. */
	UPROPERTY(EditAnywhere, Category = "DragonBoss|Skill Pick", meta = (ClampMin = "0"))
	int32 RandomSkillMaxInclusiveScratch = 3;

private:
	static void SelectSkill_ApplySkillByteToBlackboard(UBlackboardComponent* Blackboard,
		const FBlackboardKeySelector& SkillKeyScratch,
		const uint8 SkillByteScratch);
};
