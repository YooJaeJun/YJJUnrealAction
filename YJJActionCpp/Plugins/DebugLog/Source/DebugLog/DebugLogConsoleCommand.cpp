#include "DebugLogConsoleCommand.h"
#include "HAL/IConsoleManager.h"
#include "Misc/MessageDialog.h"
#include "GameFramework/PlayerController.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Interfaces/IMainFrameModule.h"

FDebugLogConsoleCommand::FDebugLogConsoleCommand()
{
	const FConsoleCommandDelegate command_Debugf = 
		FConsoleCommandDelegate::CreateRaw(
			this, 	&FDebugLogConsoleCommand::ExecuteCommand_ViewMessageDialog);

	const FConsoleCommandWithArgsDelegate command_OpenSWindow =
		FConsoleCommandWithArgsDelegate::CreateRaw(
			this, &FDebugLogConsoleCommand::ExecuteCommand_OpenSWindow);

	const FConsoleCommandWithWorldAndArgsDelegate command_Teleport =
		FConsoleCommandWithWorldAndArgsDelegate::CreateRaw(
			this, &FDebugLogConsoleCommand::ExecuteCommand_Teleport);

	Command_ViewMessageDialog = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("YJJViewMessageDialog"),
		TEXT("View Message Dialog"), 
		command_Debugf);

	Command_OpenSWindow = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("YJJOpenSWindow"),
		TEXT("Open SWindow"),
		command_OpenSWindow);

	Command_Teleport = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("YJJTeleport"),
		TEXT("Set player location to location input. If Empty, Set location to PlayerStart"),
		command_Teleport);
}

FDebugLogConsoleCommand::~FDebugLogConsoleCommand()
{
	if (!!Command_ViewMessageDialog)
		IConsoleManager::Get().UnregisterConsoleObject(Command_ViewMessageDialog);

	if (!!Command_OpenSWindow)
		IConsoleManager::Get().UnregisterConsoleObject(Command_OpenSWindow);

	if (!!Command_Teleport)
		IConsoleManager::Get().UnregisterConsoleObject(Command_Teleport);
}

void FDebugLogConsoleCommand::ExecuteCommand_ViewMessageDialog() const
{
	const FText title = FText::FromString("YJJ Debug Log");
	const FText context = FText::FromString("This command is test log.");

	FMessageDialog::Debugf(context, &title);
}

void FDebugLogConsoleCommand::ExecuteCommand_OpenSWindow(const TArray<FString>& InArgs) const
{
	FString split;
	split.Append("InArgs : ");
	split.Append(FString::FromInt(InArgs.Num()));
	split.Append("||");
	split.Append("FApp::IsGame : ");
	split.Append(FApp::IsGame() ? "True" : "False");
	split.Append("||");

	for (const FString& args : InArgs)
	{
		split.Append(args);
		split.Append("||");
	}

	OpenSWindow(split);
}

void FDebugLogConsoleCommand::ExecuteCommand_Teleport(const TArray<FString>& InArgs, UWorld* InWorld) const
{
	if (false == FApp::IsGame())
		return;

	const TWeakObjectPtr<AController> controller = InWorld->GetFirstPlayerController();
	if (false == controller.IsValid())
		return;

	const TWeakObjectPtr<ACPlayableCharacter> player = Cast<ACPlayableCharacter>(controller->GetPawn());
	if (false == player.IsValid())
		return;

	if (InArgs.Num() == 3)
	{
		FVector position;
		position.X = FCString::Atof(*InArgs[0]);
		position.Y = FCString::Atof(*InArgs[1]);
		position.Z = FCString::Atof(*InArgs[2]);

		player->SetActorLocation(position);

		return;
	}

	for (const TWeakObjectPtr<AActor> actor : InWorld->GetCurrentLevel()->Actors)
	{
		if (actor.IsValid() && actor->GetName().Contains("PlayerStart"))
			player->SetActorLocation(actor->GetActorLocation());
	}
}

void FDebugLogConsoleCommand::OpenSWindow(FString& InString) const
{
	const TSharedRef<SVerticalBox> box = SNew(SVerticalBox);

	TArray<FString> splits;
	InString.ParseIntoArray(splits, L"||");

	for (const FString& split : splits)
	{
		box.Get().AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(split))
		];
	}

	const TSharedRef<SWindow> window = SNew(SWindow)
	.Title(FText::FromString("YJJ Console Command Window"))
	.ClientSize(FVector2D(640, 480))
	.SupportsMaximize(false)
	.SupportsMinimize(false)
	.Content()
	[
		box
	];


	// MainFrameModule == 엔진 에디터 기본 UI
	const IMainFrameModule& mainFrame = 
		FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");

	if (mainFrame.GetParentWindow().IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(
			window, 
			mainFrame.GetParentWindow().ToSharedRef());
		return;
	}

	FSlateApplication::Get().AddWindow(window);
}
