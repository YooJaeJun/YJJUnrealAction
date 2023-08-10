#include "DebugLogConsoleCommand.h"
#include "HAL/IConsoleManager.h"
#include "Misc/MessageDialog.h"
#include "GameFramework/PlayerController.h"
#include "Characters/Player/CPlayableCharacter.h"

FDebugLogConsoleCommand::FDebugLogConsoleCommand()
{
	const FConsoleCommandDelegate command_Debugf = 
		FConsoleCommandDelegate::CreateRaw(this, 
			&FDebugLogConsoleCommand::ExecuteCommand_ViewMessageDialog);

	const FConsoleCommandWithWorldAndArgsDelegate command_Teleport =
		FConsoleCommandWithWorldAndArgsDelegate::CreateRaw(
			this, &FDebugLogConsoleCommand::ExecuteCommand_Teleport);

	Command_ViewMessageDialog = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("YJJViewMessageDialog"),
		TEXT("YJJ View Message Dialog"), 
		command_Debugf);

	Command_Teleport = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("YJJTeleport"),
		TEXT("Set player location to PlayerStart"),
		command_Teleport);
}

FDebugLogConsoleCommand::~FDebugLogConsoleCommand()
{
	if (!!Command_ViewMessageDialog)
		IConsoleManager::Get().UnregisterConsoleObject(Command_ViewMessageDialog);

	if (!!Command_Teleport)
		IConsoleManager::Get().UnregisterConsoleObject(Command_Teleport);
}

void FDebugLogConsoleCommand::ExecuteCommand_ViewMessageDialog() const
{
	const FText title = FText::FromString("YJJ Debug Log");
	const FText context = FText::FromString("This command is test log.");

	FMessageDialog::Debugf(context, &title);
}

void FDebugLogConsoleCommand::ExecuteCommand_Teleport(const TArray<FString>& InArgs, UWorld* InWorld) const
{
	const TWeakObjectPtr<AController> controller = InWorld->GetFirstPlayerController();
	if (false == controller.IsValid())
		return;

	const TWeakObjectPtr<ACPlayableCharacter> player = Cast<ACPlayableCharacter>(controller->GetPawn());
	if (false == player.IsValid())
		return;

	for (const TWeakObjectPtr<AActor> actor : InWorld->GetCurrentLevel()->Actors)
	{
		if (actor.IsValid() && actor->GetName().Contains("PlayerStart"))
			player->SetActorLocation(actor->GetActorLocation());
	}
}
