#include "SActData.h"
#include "WeaponStyle.h"
#include "IPropertyUtilities.h"
#include "IDetailPropertyRow.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"

TSharedRef<IPropertyTypeCustomization> SActData::MakeInstance()
{
	return MakeShareable(new SActData());
}

void SActData::CustomizeHeader(
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
	.MaxDesiredWidth(FWeaponStyle::Get()->DesiredWidth.Y)
	[
		InPropertyHandle->CreatePropertyValueWidget()
	];

	const int32 index = InPropertyHandle->GetIndexInArray();

	FString name = InPropertyHandle->GetPropertyDisplayName().ToString();
	name = "Act Data - " + name;

	InHeaderRow
	.NameContent()
	[
		SNew(SBorder)
		.BorderImage(FWeaponStyle::Get()->Array_Image.Get())
		[
			InPropertyHandle->CreatePropertyNameWidget(FText::FromString(name))
		]
	];
}

void SActData::CustomizeChildren(
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

		TSharedPtr<SWidget> name;
		TSharedPtr<SWidget> value;

		row.GetDefaultWidgets(name, value);

		row.CustomWidget()
		.NameContent()
		[
			name.ToSharedRef()
		]
		.ValueContent()
		.MinDesiredWidth(FWeaponStyle::Get()->DesiredWidth.X)
		.MaxDesiredWidth(FWeaponStyle::Get()->DesiredWidth.Y)
		[
			value.ToSharedRef()
		];
	}//for(i)
}
