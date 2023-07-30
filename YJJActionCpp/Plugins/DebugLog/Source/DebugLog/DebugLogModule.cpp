#include "DebugLogModule.h"
#include "GameplayDebugger.h"
#include "DebugLogCategory.h"
#include "DebugLogConsoleCommand.h"

#define LOCTEXT_NAMESPACE "FDebugLogModule"
	
IMPLEMENT_MODULE(FDebugLogModule, DebugLog)

void FDebugLogModule::StartupModule()
{
	IGameplayDebugger::FOnGetCategory category;
	category.BindStatic(&FDebugLogCategory::MakeInstance);

	// Debugger
	IGameplayDebugger::Get().RegisterCategory(
		"YJJDebugLog",
		category,
		EGameplayDebuggerCategoryState::EnabledInGameAndSimulate,
		5);

	// Console Command
	{
		// Shareable: 둘다생성가능
		// Shared: 기존 포인터가 있는 상황에서 shared 포인터 생성 시
		// SharedRef: shared 레퍼런스 생성 시
		ConsoleCommand = MakeShareable(new FDebugLogConsoleCommand());
	}
}

void FDebugLogModule::ShutdownModule()
{
	if (IGameplayDebugger::IsAvailable())
		IGameplayDebugger::Get().UnregisterCategory("YJJDebugLog");

	if (ConsoleCommand.IsValid())
		ConsoleCommand.Reset();
}

#undef LOCTEXT_NAMESPACE