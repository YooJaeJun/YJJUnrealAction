#include "CCommonCharacter.h"
#include "Global.h"

ACCommonCharacter::ACCommonCharacter()
{
	CHelpers::CreateActorComponent<UCStateComponent>(this, &StateComp, "StateComponent");
	CHelpers::CreateActorComponent<UCMovementComponent>(this, &MovementComp, "MovementComponent");
	CHelpers::CreateActorComponent<UCCharacterInfoComponent>(this, &CharacterInfoComp, "CharacterInfoComponent");
	CHelpers::CreateActorComponent<UCCharacterStatComponent>(this, &CharacterStatComp, "CharacterStatComponent");

	MyCurController = GetController();
}

void ACCommonCharacter::BeginPlay()
{
	Super::BeginPlay();

	MovementComp->SetSpeed(ESpeedType::Sprint);
}