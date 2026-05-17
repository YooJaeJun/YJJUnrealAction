#include "Characters/Animals/Dragon/CBTTask_DragonBoss_SkillBranch.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Characters/Animals/CAnimalWeapon.h"
#include "Characters/Animals/Dragon/CDragon.h"
#include "Characters/Animals/Dragon/Weapon/CDragonWeapon.h"
#include "Commons/CEnums.h"
#include "Components/CStateComponent.h"
#include "GameFramework/Pawn.h"
#include "Utilities/YJJHelpers.h"

namespace
{
	/** 레거시 블프는 DragonWeapon.InAction 과 무기 Weapon.InAction 을 OR 로 묶었음 — 종료 조건 재현. */
	static bool DragonBoss_IsAnyEquippedWeaponBusy(ACDragon* DragonScratch)
	{
		if (false == IsValid(DragonScratch))
		{
			return false;
		}

		bool anyBusyScratch = false;

		ACDragonWeapon* const dragonWeaponScratch = Cast<ACDragonWeapon>(DragonScratch->DragonWeapon.Get());
		if (IsValid(dragonWeaponScratch))
		{
			anyBusyScratch = anyBusyScratch || dragonWeaponScratch->InAction;
		}

		ACAnimalWeapon* const animalWeaponScratch = Cast<ACAnimalWeapon>(DragonScratch->AnimalWeapon.Get());
		if (IsValid(animalWeaponScratch) && animalWeaponScratch != static_cast<ACAnimalWeapon*>(dragonWeaponScratch))
		{
			anyBusyScratch = anyBusyScratch || animalWeaponScratch->InAction;
		}

		return anyBusyScratch;
	}
}

void UCBTTask_DragonBoss_SkillBranch::DragonBoss_ApplyAbortOneWeapon(ACAnimalWeapon* WeaponScratch)
{
	if (false == IsValid(WeaponScratch))
	{
		return;
	}

	if (WeaponScratch->InAction)
	{
		WeaponScratch->End_DoAction();
		return;
	}

	WeaponScratch->Begin_DoAction();
	WeaponScratch->End_DoAction();
}

UCBTTask_DragonBoss_SkillBranch::UCBTTask_DragonBoss_SkillBranch()
{
	bNotifyTick = true;
	NodeName = TEXT("Dragon Boss Skill Branch");
}

void UCBTTask_DragonBoss_SkillBranch::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	const UBlackboardData* const bbScratch = Asset.BlackboardAsset;
	if (nullptr == bbScratch)
	{
		return;
	}

	SkillTypeKey.ResolveSelectedKey(*bbScratch);
	SkillWaitTimeKey.ResolveSelectedKey(*bbScratch);
}

EBTNodeResult::Type UCBTTask_DragonBoss_SkillBranch::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	(void)NodeMemory;

	AAIController* const aiScratch = OwnerComp.GetAIOwner();
	if (false == IsValid(aiScratch))
	{
		UE_LOG(LogBehaviorTree,
			Error,
			TEXT("[UCBTTask_DragonBoss_SkillBranch] AIController 유효하지 않음 — BT 소유 검증 필요."));
		return EBTNodeResult::Failed;
	}

	APawn* const pawnScratch = aiScratch->GetPawn();

	ACDragon* const dragonScratch = Cast<ACDragon>(pawnScratch);
	if (false == IsValid(dragonScratch))
	{
		UE_LOG(LogBehaviorTree,
			Error,
			TEXT("[UCBTTask_DragonBoss_SkillBranch] ControlledPawn 이 ACDragon 이 아님 — 드래곤 BT 에만 넣어야 한다."));
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* const bbScratch = OwnerComp.GetBlackboardComponent();
	if (false == IsValid(bbScratch))
	{
		UE_LOG(LogBehaviorTree, Error, TEXT("[UCBTTask_DragonBoss_SkillBranch] BlackboardComponent 없음."));
		return EBTNodeResult::Failed;
	}

	if (false == SkillTypeKey.IsSet() || SkillTypeKey.SelectedKeyName == NAME_None)
	{
		UE_LOG(LogBehaviorTree,
			Error,
			TEXT("[UCBTTask_DragonBoss_SkillBranch] Skill Type 블랙보드 키가 비어 있습니다."));
		return EBTNodeResult::Failed;
	}

	if (false == SkillWaitTimeKey.IsSet() || SkillWaitTimeKey.SelectedKeyName == NAME_None)
	{
		UE_LOG(LogBehaviorTree,
			Error,
			TEXT("[UCBTTask_DragonBoss_SkillBranch] Skill Wait Time 블랙보드 키가 비어 있습니다."));
		return EBTNodeResult::Failed;
	}

	const uint8 skillByteScratch = bbScratch->GetValueAsEnum(SkillTypeKey.SelectedKeyName);

	if (skillByteScratch >= static_cast<uint8>(CEDragonBossSkillType::Max))
	{
		UE_LOG(LogBehaviorTree,
			Error,
			TEXT("[UCBTTask_DragonBoss_SkillBranch] 스킬 enum 바이트(%u)가 CEDragonBossSkillType::Max 보다 크거나 같다."),
			static_cast<uint32>(skillByteScratch));
		return EBTNodeResult::Failed;
	}

	aiScratch->StopMovement();

	dragonScratch->InvokeDragonWeaponSkill(skillByteScratch);

	float waitScratch = FallbackSkillWaitSeconds;

	const ACDragonWeapon* const weaponTypedScratch = Cast<ACDragonWeapon>(dragonScratch->DragonWeapon.Get());
	if (IsValid(weaponTypedScratch) && weaponTypedScratch->SkillWaitTime.Num() > 0)
	{
		const int32 upperScratch = weaponTypedScratch->SkillWaitTime.Num() - 1;
		const int32 indexScratch =
			FMath::Clamp(static_cast<int32>(skillByteScratch), 0, upperScratch);

		if (weaponTypedScratch->SkillWaitTime.IsValidIndex(indexScratch))
		{
			waitScratch = static_cast<float>(weaponTypedScratch->SkillWaitTime[indexScratch]);
		}
	}

	bbScratch->SetValueAsFloat(SkillWaitTimeKey.SelectedKeyName, waitScratch);

	return EBTNodeResult::InProgress;
}

void UCBTTask_DragonBoss_SkillBranch::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	(void)NodeMemory;
	(void)DeltaSeconds;

	AAIController* const aiScratch = OwnerComp.GetAIOwner();
	if (false == IsValid(aiScratch))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* const pawnScratch = aiScratch->GetPawn();
	ACDragon* const dragonScratch = Cast<ACDragon>(pawnScratch);
	if (false == IsValid(dragonScratch))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UCStateComponent* const stateScratch = YJJHelpers::GetComponent<UCStateComponent>(dragonScratch);
	if (false == IsValid(stateScratch))
	{
		UE_LOG(LogBehaviorTree,
			Error,
			TEXT("[UCBTTask_DragonBoss_SkillBranch] UCStateComponent 없음 — 종료 처리(%s)."),
			*GetNameSafe(dragonScratch));
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (false == stateScratch->IsIdle())
	{
		return;
	}

	const bool weaponBusyScratch = DragonBoss_IsAnyEquippedWeaponBusy(dragonScratch);
	if (false == weaponBusyScratch)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UCBTTask_DragonBoss_SkillBranch::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* const aiScratch = OwnerComp.GetAIOwner();

	if (IsValid(aiScratch))
	{
		ACDragon* const dragonScratch = Cast<ACDragon>(aiScratch->GetPawn());

		if (IsValid(dragonScratch))
		{
			ACDragonWeapon* const dragonWeaponScratch = Cast<ACDragonWeapon>(dragonScratch->DragonWeapon.Get());
			DragonBoss_ApplyAbortOneWeapon(dragonWeaponScratch);

			ACAnimalWeapon* const animalWeaponScratch = Cast<ACAnimalWeapon>(dragonScratch->AnimalWeapon.Get());
			if (IsValid(animalWeaponScratch)
				&& animalWeaponScratch != static_cast<ACAnimalWeapon*>(dragonWeaponScratch))
			{
				DragonBoss_ApplyAbortOneWeapon(animalWeaponScratch);
			}
		}
	}

	return Super::AbortTask(OwnerComp, NodeMemory);
}
