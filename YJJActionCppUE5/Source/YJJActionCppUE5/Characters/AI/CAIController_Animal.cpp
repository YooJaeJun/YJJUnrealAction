#include "Characters/AI/CAIController_Animal.h"

#include "BehaviorTree/BehaviorTree.h"
#include "Characters/Animals/CAnimal.h"
#include "Perception/AISenseConfig_Sight.h"

ACAIController_Animal::ACAIController_Animal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void ACAIController_Animal::OnPossess(APawn* InPawn)
{
	Animal_AI = Cast<ACAnimal>(InPawn);
	Super::OnPossess(InPawn);
}

void ACAIController_Animal::OnUnPossess()
{
	Animal_AI = nullptr;
	Super::OnUnPossess();
}

bool ACAIController_Animal::ShouldUseHearingPerceptionSense() const
{
	return false;
}

void ACAIController_Animal::ConfigureSightSenseAffiliation(UAISenseConfig_Sight& SightConfigRef)
{
	// BP export: DetectionByAffiliation 에 Neutrals + Friendlies 만 명시(Enemy 탐지 끔).
	SightConfigRef.DetectionByAffiliation.bDetectEnemies = false;
	SightConfigRef.DetectionByAffiliation.bDetectFriendlies = true;
	SightConfigRef.DetectionByAffiliation.bDetectNeutrals = true;
}
