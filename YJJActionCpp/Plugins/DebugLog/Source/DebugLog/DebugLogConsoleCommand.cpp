#include "DebugLogConsoleCommand.h"
#include "HAL/IConsoleManager.h"
#include "Misc/MessageDialog.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

FDebugLogConsoleCommand::FDebugLogConsoleCommand()
{
	const FConsoleCommandDelegate command = 
		FConsoleCommandDelegate::CreateRaw(this, &FDebugLogConsoleCommand::ExecuteCommand);

	Command = IConsoleManager::Get().RegisterConsoleCommand(
		L"YJJViewMessageDialog",
		L"YJJ View Message Dialog", 
		command);

}

FDebugLogConsoleCommand::~FDebugLogConsoleCommand()
{
	if (!!Command)
		IConsoleManager::Get().UnregisterConsoleObject(Command);
}

void FDebugLogConsoleCommand::ExecuteCommand()
{
	const FText title = FText::FromString("YJJDebugLog");
	const FText context = FText::FromString("YJJDebugLog Test");

	FMessageDialog::Debugf(context, &title);
}