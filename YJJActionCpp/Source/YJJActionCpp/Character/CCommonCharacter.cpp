#include "CCommonCharacter.h"
#include "Global.h"
#include "Component/CMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ACCommonCharacter::ACCommonCharacter()
{
	CHelpers::CreateActorComponent<UCStateComponent>(this, &StateComponent, "StateComponent");
	CHelpers::CreateActorComponent<UCMovementComponent>(this, &MovementComponent, "MovementComponent");

	MyCurController = GetController();
}

void ACCommonCharacter::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent->SetSpeed(ESpeedType::Sprint);
}