#include "SWeaponDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "Weapons/CWeaponAsset.h"
#include "NiagaraSystem.h"

TSharedRef<IDetailCustomization> SWeaponDetailsView::MakeInstance()
{
	return MakeShareable(new SWeaponDetailsView());
}

void SWeaponDetailsView::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UClass* type = UCWeaponAsset::StaticClass();

	//DetailBuilder.HideCategory("CWeaponAsset");

	//Class Settings
	{
		IDetailCategoryBuilder& category = 
			DetailBuilder.EditCategory("ClassSettings", FText::FromString("Class Settings"));
		category.AddProperty("AttachmentClass", type);
		category.AddProperty("EquipmentClass", type);
		category.AddProperty("ActClass", type);
		category.AddProperty("SkillClass", type);
	}

	//EquipmentData
	{
		IDetailCategoryBuilder& category = 
			DetailBuilder.EditCategory("EquipmentData", FText::FromString("Equipment Data"));
		IDetailPropertyRow& row = category.AddProperty("EquipmentData", type);
	}

	//ActData
	{
		IDetailCategoryBuilder& category = 
			DetailBuilder.EditCategory("ActData", FText::FromString("Act Data"));
		IDetailPropertyRow& row = category.AddProperty("ActDatas", type);
	}

	//HitData
	{
		IDetailCategoryBuilder& category = 
			DetailBuilder.EditCategory("HitData", FText::FromString("Hit Data"));
		IDetailPropertyRow& row = category.AddProperty("HitDatas", type);
	}
}