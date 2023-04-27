#include "CCharacter.h"
#include "Global.h"
#include "Component/CStateComponent.h"
#include "Component/CMovingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ACCharacter::ACCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CHelpers::CreateActorComponent<UCStateComponent>(this, &StateComponent, "StateComponent");
	CHelpers::CreateActorComponent<UCMovingComponent>(this, &MovingComponent, "MovingComponent");
}

void ACCharacter::BeginPlay()
{
	Super::BeginPlay();

	MovingComponent->SetMaxWalkSpeed(ESpeedType::Sprint);
}

void ACCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}