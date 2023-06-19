#include "Characters/AI/CAIController.h"
#include "Global.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"

const FName ACAIController::SelfActor(TEXT("SelfActor"));
const FName ACAIController::Target(TEXT("Target"));
const FName ACAIController::Behavior(TEXT("Behavior"));
const FName ACAIController::TargetLocation(TEXT("TargetLocation"));

ACAIController::ACAIController()
{
	CHelpers::GetAsset(&BBAsset, "BlackboardData'/Game/Character/Enemies/CBB_Enemy.CBB_Enemy'");
	CHelpers::GetAsset(&BTAsset, "BehaviorTree'/Game/Character/Enemies/Melee/CBT_Melee.CBT_Melee'");
}

void ACAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	RunAI();
}

void ACAIController::RunAI()
{
	if (UseBlackboard(BBAsset, Blackboard))
		RunBehaviorTree(BTAsset);
}

void ACAIController::StopAI() const
{
	const TWeakObjectPtr<UBehaviorTreeComponent> behaviorTreeComponent =
		Cast<UBehaviorTreeComponent>(BrainComponent);

	CheckNull(behaviorTreeComponent);

	behaviorTreeComponent->StopTree(EBTStopMode::Safe);
}
