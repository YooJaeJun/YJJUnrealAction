#include "CCommonCharacter.h"
#include "Global.h"

ACCommonCharacter::ACCommonCharacter()
{
	CHelpers::CreateActorComponent<UCStateComponent>(this, &StateComponent, "StateComponent");
	CHelpers::CreateActorComponent<UCMovementComponent>(this, &MovementComponent, "MovementComponent");
	CHelpers::CreateActorComponent<UCCharacterInfoComponent>(this, &CharacterInfoComponent, "CharacterInfoComponent");
	CHelpers::CreateActorComponent<UCCharacterStatComponent>(this, &CharacterStatComponent, "CharacterStatComponent");

	MyCurController = GetController();
}

void ACCommonCharacter::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent->SetSpeed(ESpeedType::Sprint);
}