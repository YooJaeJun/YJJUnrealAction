#include "Characters/AI/CAIController.h"
#include "Global.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "Components/CCharacterInfoComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

const FName ACAIController::SelfActor(TEXT("SelfActor"));
const FName ACAIController::Target(TEXT("Target"));
const FName ACAIController::Behavior(TEXT("Behavior"));
const FName ACAIController::TargetLocation(TEXT("TargetLocation"));

ACAIController::ACAIController()
{
	YJJHelpers::GetAsset(&BBAsset, "BlackboardData'/Game/Character/Enemies/CBB_Enemy.CBB_Enemy'");
	YJJHelpers::GetAsset(&BTAsset, "BehaviorTree'/Game/Character/Enemies/Melee/CBT_Melee.CBT_Melee'");
	YJJHelpers::CreateActorComponent<UAIPerceptionComponent>(this, &PerceptionComponent, "Perception");
	YJJHelpers::CreateActorComponent<UAISenseConfig_Sight>(this, &SightConfig, "SenseSight");
	YJJHelpers::CreateActorComponent<UAISenseConfig_Hearing>(this, &HearingConfig, "SenseHearing");

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = Angle;
	SightConfig->SetMaxAge(5.0f);

	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->SetMaxAge(5.0f);

	PerceptionComponent->ConfigureSense(*SightConfig);
	PerceptionComponent->ConfigureSense(*HearingConfig);
	PerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACAIController::OnTargetDetected);
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

void ACAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	CheckNull(GetPawn());

	const TWeakObjectPtr<UCCharacterInfoComponent> infoComp =
		YJJHelpers::GetComponent<UCCharacterInfoComponent>(Cast<ACCommonCharacter>(GetPawn()));
	CheckNull(infoComp);

	const TWeakObjectPtr<ACCommonCharacter> other = Cast<ACCommonCharacter>(Actor);
	CheckNull(other);

	CheckTrue(infoComp->IsSameGroup(other));

	Blackboard->SetValueAsObject(Target, other.Get());
}
