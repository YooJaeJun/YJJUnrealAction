#pragma once

#include "CoreMinimal.h"

class DEBUGLOG_API FDebugLogConsoleCommand
{
public:
	FDebugLogConsoleCommand();
	~FDebugLogConsoleCommand();

private:
	// 꼭 해제할 것 (UnregisterConsoleObject)
	IConsoleCommand* Command_ViewMessageDialog;
	IConsoleCommand* Command_OpenSWindow;
	IConsoleCommand* Command_Teleport;

private:
	void ExecuteCommand_ViewMessageDialog() const;
	void ExecuteCommand_OpenSWindow(const TArray<FString>& InArgs) const;
	void ExecuteCommand_Teleport(const TArray<FString>& InArgs, UWorld* InWorld) const;

	void OpenSWindow(FString& InString) const;
};
