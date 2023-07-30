#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDebugLogConsoleCommand;

class FDebugLogModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FDebugLogConsoleCommand> ConsoleCommand;
};
