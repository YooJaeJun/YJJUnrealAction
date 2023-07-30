#pragma once

#include "CoreMinimal.h"

class DEBUGLOG_API FDebugLogConsoleCommand
{
public:
	FDebugLogConsoleCommand();
	~FDebugLogConsoleCommand();

private:
	struct IConsoleCommand* Command;

private:
	void ExecuteCommand();
	void ExecuteCommand3(const TArray<FString>& InArgs, UWorld* InWorld);
};
