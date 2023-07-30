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
		// Shareable: �Ѵٻ�������
		// Shared: ���� �����Ͱ� �ִ� ��Ȳ���� shared ������ ���� ��
		// SharedRef: shared ���۷��� ���� ��
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