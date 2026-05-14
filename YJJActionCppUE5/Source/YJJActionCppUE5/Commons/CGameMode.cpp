#include "Commons/CGameMode.h"
#include "Global.h"
#include "Commons/CGameState.h"
#include "Commons/CPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

ACGameMode::ACGameMode()
{
	YJJHelpers::GetClass<APawn>(&DefaultPawnClass, "/Script/Engine.Blueprint'/Game/Character/Player/CBP_PlayableCharacter.CBP_PlayableCharacter_C'");

	PlayerControllerClass = ACPlayerController::StaticClass();
	GameStateClass = ACGameState::StaticClass();
}

void ACGameMode::BeginPlay()
{
	Super::BeginPlay();
}

UCUserWidget_HUD* ACGameMode::GetHUD() const
{
	const UWorld* world = GetWorld();
	if (false == IsValid(world))
		return nullptr;

	for (FConstPlayerControllerIterator iterator = world->GetPlayerControllerIterator(); iterator; ++iterator)
	{
		APlayerController* playerController = iterator->Get();
		if (false == IsValid(playerController))
			continue;

		if (false == playerController->IsLocalController())
			continue;

		ACPlayerController* yjjPc = Cast<ACPlayerController>(playerController);
		if (false == IsValid(yjjPc))
			continue;

		return yjjPc->EnsureHUD();
	}

	return nullptr;
}
