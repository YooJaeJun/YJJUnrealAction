#include "SEquipmentData.h"
#include "WeaponStyle.h"
#include "IPropertyUtilities.h"
#include "IDetailPropertyRow.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"

TSharedRef<IPropertyTypeCustomization> SEquipmentData::MakeInstance()
{
	return MakeShareable(new SEquipmentData());
}

void SEquipmentData::CustomizeHeader(
	TSharedRef<IPropertyHandle> InPropertyHandle, 
	FDetailWidgetRow& InHeaderRow,
	IPropertyTypeCustomizationUtils& InCustomizationUtils)
{
	InHeaderRow
	.NameContent()
	[
		InPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(FWeaponStyle::Get()->DesiredWidth.X)
	.MaxDesiredWidth(FWeaponStyle::Get()->DesiredWidth.Y);
}

void SEquipmentData::CustomizeChildren(
	TSharedRef<IPropertyHandle> InPropertyHandle,
	IDetailChildrenBuilder& InChildBuilder, 
	IPropertyTypeCustomizationUtils& InCustomizationUtils)
{
	uint32 number = 0;
	InPropertyHandle->GetNumChildren(number);

	for (uint32 i = 0; i < number; i++)
	{
		TSharedPtr<IPropertyHandle> handle = InPropertyHandle->GetChildHandle(i);
		IDetailPropertyRow& row = InChildBuilder.AddProperty(handle.ToSharedRef());

		row.CustomWidget()
		.NameContent()
		[
			handle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(FWeaponStyle::Get()->DesiredWidth.X)
		.MaxDesiredWidth(FWeaponStyle::Get()->DesiredWidth.Y)
		[
			handle->CreatePropertyValueWidget()
		];
	}
}
