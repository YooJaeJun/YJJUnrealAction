#include "Characters/Animals/CBTTask_AnimalAction.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Animals/CAnimal.h"
#include "Characters/Animals/CAnimalWeapon.h"
#include "Characters/Animals/Dragon/CDragon.h"
#include "Components/CStateComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Utilities/YJJHelpers.h"

namespace
{
	static ACAnimalWeapon* ResolveAnimalWeaponForBehaviorTask(ACAnimal* OwningAnimal)
	{
		if (nullptr == OwningAnimal)
		{
			return nullptr;
		}

		AActor* weaponActorLocal = OwningAnimal->AnimalWeapon.Get();
		if (nullptr != weaponActorLocal)
		{
			ACAnimalWeapon* const typedLocal = Cast<ACAnimalWeapon>(weaponActorLocal);
			if (IsValid(typedLocal))
			{
				return typedLocal;
			}
		}

		ACDragon* const dragonLocal = Cast<ACDragon>(OwningAnimal);

		if (false == IsValid(dragonLocal))
		{
			return nullptr;
		}

		weaponActorLocal = dragonLocal->DragonWeapon.Get();

		return Cast<ACAnimalWeapon>(weaponActorLocal);
	}

	static void AnimalAction_ApplyAbortWeaponCleanup(ACAnimal* Animal)
	{
		if (false == IsValid(Animal))
		{
			return;
		}

		ACAnimalWeapon* weaponLocal = ResolveAnimalWeaponForBehaviorTask(Animal);

		if (false == IsValid(weaponLocal))
		{
			return;
		}

		// 레거시 Abort: InAction 면 End 만, 아니면 Begin 후 End 로 무기 상태를 한 번 재동기화.
		if (weaponLocal->InAction)
		{
			weaponLocal->End_DoAction();
			return;
		}

		weaponLocal->Begin_DoAction();
		weaponLocal->End_DoAction();
	}
} // namespace

UCBTTask_AnimalAction::UCBTTask_AnimalAction()
{
	bNotifyTick = true;
	NodeName = TEXT("Animal Action");

	TargetBlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UCBTTask_AnimalAction, TargetBlackboardKey), AActor::StaticClass());
}

EBTNodeResult::Type UCBTTask_AnimalAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* const aiController = OwnerComp.GetAIOwner();
	if (false == IsValid(aiController))
	{
		return EBTNodeResult::Failed;
	}

	APawn* const controlledPawn = aiController->GetPawn();

	ACAnimal* const animalScratch = Cast<ACAnimal>(controlledPawn);
	if (false == IsValid(animalScratch))
	{
		return EBTNodeResult::Failed;
	}

	ACAnimalWeapon* const weaponScratch = ResolveAnimalWeaponForBehaviorTask(animalScratch);
	if (false == IsValid(weaponScratch))
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* const blackboardScratch = OwnerComp.GetBlackboardComponent();
	if (false == IsValid(blackboardScratch))
	{
		return EBTNodeResult::Failed;
	}

	if (TargetBlackboardKey.SelectedKeyName == NAME_None)
	{
		UE_LOG(LogBehaviorTree,
			Error,
			TEXT("[UCBTTask_AnimalAction] 블랙보드 Target 키 미선택 — BT 노드 디테일에서 Target 을 블랙보드에 연결했는지 확인."));
		return EBTNodeResult::Failed;
	}

	AActor* const targetScratch = Cast<AActor>(blackboardScratch->GetValueAsObject(TargetBlackboardKey.SelectedKeyName));
	if (false == IsValid(targetScratch))
	{
		return EBTNodeResult::Failed;
	}

	aiController->StopMovement();

	weaponScratch->DoAction(targetScratch->GetActorLocation());

	return EBTNodeResult::InProgress;
}

void UCBTTask_AnimalAction::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

	ACAnimal* const animalScratch = Cast<ACAnimal>(pawnScratch);
	if (false == IsValid(animalScratch))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UCStateComponent* const stateScratch = YJJHelpers::GetComponent<UCStateComponent>(animalScratch);
	ACAnimalWeapon* const weaponScratch = ResolveAnimalWeaponForBehaviorTask(animalScratch);

	if ((false == IsValid(stateScratch)) || (false == IsValid(weaponScratch)))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 깨졌던 블프는 Pawn 에서 특정 컴포넌트 IsIdle 노드를 찾았다 — 동물 계열 공통 소스는 UCStateComponent::IsIdle.
	const bool notInActionScratch = false == weaponScratch->InAction;
	const bool idleScratch = stateScratch->IsIdle();

	const bool terminalScratch = notInActionScratch && idleScratch;
	if (terminalScratch)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}

EBTNodeResult::Type UCBTTask_AnimalAction::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* const aiScratch = OwnerComp.GetAIOwner();
	if (false == IsValid(aiScratch))
	{
		return Super::AbortTask(OwnerComp, NodeMemory);
	}

	APawn* const pawnScratch = aiScratch->GetPawn();
	ACAnimal* const animalScratch = Cast<ACAnimal>(pawnScratch);
	if (false == IsValid(animalScratch))
	{
		return Super::AbortTask(OwnerComp, NodeMemory);
	}

	AnimalAction_ApplyAbortWeaponCleanup(animalScratch);

	return Super::AbortTask(OwnerComp, NodeMemory);
}
