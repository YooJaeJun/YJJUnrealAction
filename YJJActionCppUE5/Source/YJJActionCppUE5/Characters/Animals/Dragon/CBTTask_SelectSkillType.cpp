#include "Characters/Animals/Dragon/CBTTask_SelectSkillType.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "Characters/Animals/Dragon/CDragon.h"
#include "GameFramework/Pawn.h"

UCBTTask_SelectSkillType::UCBTTask_SelectSkillType()
{
	NodeName = TEXT("Select Skill Type");
}

void UCBTTask_SelectSkillType::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	const UBlackboardData* bbAssetScratch = Asset.BlackboardAsset;

	if (nullptr == bbAssetScratch)
	{
		return;
	}

	SkillTypeBlackboardKey.ResolveSelectedKey(*bbAssetScratch);
	DistanceTypeBlackboardKey.ResolveSelectedKey(*bbAssetScratch);
}

void UCBTTask_SelectSkillType::SelectSkill_ApplySkillByteToBlackboard(UBlackboardComponent* Blackboard,
	const FBlackboardKeySelector& SkillKeyScratch,
	const uint8 SkillByteScratch)
{
	if (false == IsValid(Blackboard))
	{
		return;
	}

	if (false == SkillKeyScratch.IsSet())
	{
		return;
	}

	Blackboard->SetValueAsEnum(SkillKeyScratch.SelectedKeyName, SkillByteScratch);
}

EBTNodeResult::Type UCBTTask_SelectSkillType::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	(void)NodeMemory;

	AAIController* const aiScratch = OwnerComp.GetAIOwner();
	if (false == IsValid(aiScratch))
	{
		return EBTNodeResult::Failed;
	}

	APawn* const pawnScratch = aiScratch->GetPawn();

	ACDragon* const dragonScratch = Cast<ACDragon>(pawnScratch);
	if (false == IsValid(dragonScratch))
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* const bbScratch = OwnerComp.GetBlackboardComponent();

	if (false == IsValid(bbScratch))
	{
		return EBTNodeResult::Failed;
	}

	if (false == SkillTypeBlackboardKey.IsSet())
	{
		UE_LOG(LogBehaviorTree,
			Error,
			TEXT("[UCBTTask_SelectSkillType] Skill Type 블랙보드 키 미설정."));
		return EBTNodeResult::Failed;
	}

	if (SkillTypeBlackboardKey.SelectedKeyName == NAME_None)
	{
		UE_LOG(LogBehaviorTree,
			Error,
			TEXT("[UCBTTask_SelectSkillType] Skill Type 키 이름이 비어 있습니다."));
		return EBTNodeResult::Failed;
	}

	if (false == DistanceTypeBlackboardKey.IsSet())
	{
		UE_LOG(LogBehaviorTree,
			Error,
			TEXT("[UCBTTask_SelectSkillType] Distance Type 블랙보드 키 미설정."));
		return EBTNodeResult::Failed;
	}

	if (DistanceTypeBlackboardKey.SelectedKeyName == NAME_None)
	{
		UE_LOG(LogBehaviorTree,
			Error,
			TEXT("[UCBTTask_SelectSkillType] Distance Type 키 이름이 비어 있습니다."));
		return EBTNodeResult::Failed;
	}

	const uint8 distanceByteScratch = bbScratch->GetValueAsEnum(DistanceTypeBlackboardKey.SelectedKeyName);

	const bool isAvoidScratch = distanceByteScratch == DistanceAvoidByteScratch;

	if (isAvoidScratch)
	{
		// 레거시 then: 활성 몽타주 차단 후 고정 스킬 슬롯으로 전환한다.
		dragonScratch->StopAnimMontage(nullptr);

		SelectSkill_ApplySkillByteToBlackboard(bbScratch, SkillTypeBlackboardKey, SkillTypeFixedByteWhenAvoid);

		return EBTNodeResult::Succeeded;
	}

	const int32 minScratchLocal = RandomSkillMinInclusiveScratch;
	const int32 maxScratchLocal = RandomSkillMaxInclusiveScratch;

	int32 boundedMinScratch = minScratchLocal;

	int32 boundedMaxScratch = maxScratchLocal;

	if (boundedMinScratch > boundedMaxScratch)
	{
		const int32 swapScratchLocal = boundedMinScratch;
		boundedMinScratch = boundedMaxScratch;
		boundedMaxScratch = swapScratchLocal;
	}

	const int32 rngScratchLocal = FMath::RandRange(boundedMinScratch, boundedMaxScratch);

	// 무기·스킬 UDE 바이트는 uint8 블랙보드 규격 — 초과 범위는 BT 편차 대비해서 잘라낸다.
	const int32 clippedScratchLocal =
		FMath::Clamp(rngScratchLocal, 0, 255);
	const uint8 rngByteScratchLocal = static_cast<uint8>(clippedScratchLocal);

	SelectSkill_ApplySkillByteToBlackboard(bbScratch, SkillTypeBlackboardKey, rngByteScratchLocal);

	return EBTNodeResult::Succeeded;
}
