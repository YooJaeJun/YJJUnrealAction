#include "Characters/AI/CAIController_Melee.h"
#include "Global.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"

const FName ACAIController_Melee::Target(TEXT("Target"));
const FName ACAIController_Melee::Behavior(TEXT("Behavior"));
const FName ACAIController_Melee::EQLocation(TEXT("EQLocation"));

ACAIController_Melee::ACAIController_Melee()
{
	CHelpers::GetAsset(&BBAsset, "BlackboardData'/Game/Character/Enemies/CBB_Enemy.CBB_Enemy'");
	CHelpers::GetAsset(&BTAsset, "BehaviorTree'/Game/Character/Enemies/Melee/CBT_Melee.CBT_Melee'");
}

void ACAIController_Melee::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	RunAI();
}

void ACAIController_Melee::RunAI()
{
	if (UseBlackboard(BBAsset, Blackboard))
		RunBehaviorTree(BTAsset);
}

void ACAIController_Melee::StopAI() const
{
	const TWeakObjectPtr<UBehaviorTreeComponent> behaviorTreeComponent =
		Cast<UBehaviorTreeComponent>(BrainComponent);

	CheckNull(behaviorTreeComponent);

	behaviorTreeComponent->StopTree(EBTStopMode::Safe);
}
