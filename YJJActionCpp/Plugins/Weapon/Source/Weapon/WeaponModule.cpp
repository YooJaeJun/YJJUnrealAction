#include "WeaponModule.h"
#include "WeaponContextMenu.h"
#include "WeaponStyle.h"
#include "WeaponCommand.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "FWeaponModule"

// 모듈을 C++ 클래스로 구현하는 매크로.
// 모듈의 시작과 종료를 관리, 모듈의 이름과 타입을 정의.
IMPLEMENT_MODULE(FWeaponModule, Weapon)

void FWeaponModule::StartupModule()
{
	FWeaponStyle::Get();

	IAssetTools& assetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	const EAssetTypeCategories::Type categories = 
		assetTools.RegisterAdvancedAssetCategory("WeaponAsset", FText::FromString("Weapon"));

	ContextMenu = MakeShareable(new FWeaponContextMenu(categories));
	assetTools.RegisterAssetTypeActions(ContextMenu.ToSharedRef());

	Command = MakeShareable(new FWeaponCommand());
	Command->Startup();
}

void FWeaponModule::ShutdownModule()
{
	if (ContextMenu.IsValid())
		ContextMenu.Reset();

	if (Command.IsValid())
		Command.Reset();

	FWeaponStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE