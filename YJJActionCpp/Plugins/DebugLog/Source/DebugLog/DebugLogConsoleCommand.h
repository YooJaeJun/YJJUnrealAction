#pragma once

#include "CoreMinimal.h"

class DEBUGLOG_API FDebugLogConsoleCommand
{
public:
	FDebugLogConsoleCommand();
	~FDebugLogConsoleCommand();

private:
	// �� ������ �� (UnregisterConsoleObject)
	IConsoleCommand* Command_ViewMessageDialog;
	IConsoleCommand* Command_Teleport;

private:
	void ExecuteCommand_ViewMessageDialog() const;
	void ExecuteCommand_Teleport(const TArray<FString>& InArgs, UWorld* InWorld) const;
};
