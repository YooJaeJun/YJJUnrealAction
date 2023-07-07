#include "WeaponContextMenu.h"
#include "WeaponAssetEditor.h"
#include "Weapons/CWeaponAsset.h"

FWeaponContextMenu::FWeaponContextMenu(EAssetTypeCategories::Type InCategory)
{
	Category = InCategory;
}

FText FWeaponContextMenu::GetName() const
{
	return FText::FromString("DataAsset");
}

UClass* FWeaponContextMenu::GetSupportedClass() const
{
	return UCWeaponAsset::StaticClass();
}

FColor FWeaponContextMenu::GetTypeColor() const
{
	return FColor::Magenta;
}

uint32 FWeaponContextMenu::GetCategories()
{
	return Category;
}

void FWeaponContextMenu::OpenAssetEditor(
	const TArray<UObject*>& InObjects,
	TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	// �׳� Data Asset ���Ϸ� �� ���� Editor�� ����
	// FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);

	if (InObjects.Num() < 1)
		return;

	FWeaponAssetEditor::OpenWindow(InObjects[0]->GetName());
}
