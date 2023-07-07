#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FWeaponModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<class FWeaponContextMenu> ContextMenu;
	TSharedPtr<class FWeaponCommand> Command;
};
