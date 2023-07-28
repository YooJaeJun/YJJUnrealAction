#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

class WEAPON_API SActData
	: public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(
		TSharedRef<IPropertyHandle> InPropertyHandle, 
		FDetailWidgetRow& InHeaderRow, 
		IPropertyTypeCustomizationUtils& InCustomizationUtils) override;

	virtual void CustomizeChildren(
		TSharedRef<IPropertyHandle> InPropertyHandle, 
		IDetailChildrenBuilder& InChildBuilder, 
		IPropertyTypeCustomizationUtils& InCustomizationUtils) override;
};