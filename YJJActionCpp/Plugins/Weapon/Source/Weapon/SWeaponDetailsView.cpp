#include "SWeaponDetailsView.h"
#include "SWeaponCheckBoxes.h"
#include "SWeaponEquipmentData.h"
#include "SWeaponActData.h"
#include "SWeaponHitData.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "Weapons/CWeaponAsset.h"
#include "Animation/AnimMontage.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "Sound/SoundWave.h"

bool SWeaponDetailsView::bRefreshByCheckBoxes = false;

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

		if (bRefreshByCheckBoxes == false)
		{
			const TSharedPtr<SWeaponCheckBoxes> checkBoxes = SWeaponEquipmentData::CreateCheckBoxes();
			checkBoxes->AddProperties(row.GetPropertyHandle());

			FEquipmentData data;

			int32 index = 0;
			checkBoxes->CheckDefaultObject(index++, data.Montage);
			checkBoxes->CheckDefaultValue(index++, data.PlayRate);
			checkBoxes->CheckDefaultValue(index++, data.bCanMove);
			checkBoxes->CheckDefaultValue(index++, data.bUseControlRotation);
			checkBoxes->CheckDefaultObject(index++, data.Sound);
		}
	}

	//ActData
	{
		IDetailCategoryBuilder& category = 
			DetailBuilder.EditCategory("ActData", FText::FromString("Act Data"));
		IDetailPropertyRow& row = category.AddProperty("ActDatas", type);

		if (bRefreshByCheckBoxes == false)
		{
			uint32 count = 0;
			row.GetPropertyHandle()->GetNumChildren(count);

			SWeaponActData::EmptyCheckBoxes();

			FActData data;
			for (uint32 i = 0; i < count; i++)
			{
				const TSharedPtr<IPropertyHandle> handle = row.GetPropertyHandle()->GetChildHandle(i);

				const TSharedPtr<SWeaponCheckBoxes> checkBoxes = SWeaponActData::AddCheckBoxes();
				checkBoxes->AddProperties(handle);

				int32 index = 0;
				checkBoxes->CheckDefaultObject(index++, data.Montage);
				checkBoxes->CheckDefaultValue(index++, data.PlayRate);
				checkBoxes->CheckDefaultValue(index++, data.bCanMove);
				checkBoxes->CheckDefaultValue(index++, data.bUseControlRotation);
				checkBoxes->CheckDefaultValue(index++, data.bFixedCamera);
				checkBoxes->CheckDefaultObject(index++, data.Sound);
				checkBoxes->CheckDefaultObject(index++, data.Effect);
				checkBoxes->CheckDefaultValue(index++, data.EffectLocation);
				checkBoxes->CheckDefaultValue(index++, data.EffectScale);
				checkBoxes->CheckDefaultValue(index++, data.Stamina);
				checkBoxes->CheckDefaultValue(index++, data.Mana);
			}
		}//if(bRefreshByCheckBoxes)
	}

	//HitData
	{
		IDetailCategoryBuilder& category = 
			DetailBuilder.EditCategory("HitData", FText::FromString("Hit Data"));
		IDetailPropertyRow& row = category.AddProperty("HitDatas", type);

		if (bRefreshByCheckBoxes == false)
		{
			uint32 count = 0;
			row.GetPropertyHandle()->GetNumChildren(count);

			SWeaponHitData::EmptyCheckBoxes();

			FHitData data;
			for (uint32 i = 0; i < count; i++)
			{
				const TSharedPtr<IPropertyHandle> handle = row.GetPropertyHandle()->GetChildHandle(i);

				const TSharedPtr<SWeaponCheckBoxes> checkBoxes = SWeaponHitData::AddCheckBoxes();
				checkBoxes->AddProperties(handle);

				int32 index = 0;
				checkBoxes->CheckDefaultObject(index++, data.Montage);
				checkBoxes->CheckDefaultValue(index++, data.PlayRate);
				checkBoxes->CheckDefaultValue(index++, data.Power);
				checkBoxes->CheckDefaultValue(index++, data.Launch);
				checkBoxes->CheckDefaultValue(index++, data.StopTime);
				checkBoxes->CheckDefaultObject(index++, data.Sound);
				checkBoxes->CheckDefaultObject(index++, data.Effect);
				checkBoxes->CheckDefaultValue(index++, data.EffectLocation);
				checkBoxes->CheckDefaultValue(index++, data.EffectScale);
				checkBoxes->CheckDefaultValue(index++, static_cast<int32>(data.AttackType));
			}
		}//if(bRefreshByCheckBoxes)
	}
}