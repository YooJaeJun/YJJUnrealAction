#include "WeaponAssetFactory.h"
#include "Weapons/CWeaponAsset.h"

UWeaponAssetFactory::UWeaponAssetFactory()
{
	bCreateNew = true;

	SupportedClass = UCWeaponAsset::StaticClass();
}

UObject* UWeaponAssetFactory::FactoryCreateNew(
	UClass* InClass, 
	UObject* InParent,
	FName InName,
	EObjectFlags Flags,
	UObject* Context, 
	FFeedbackContext* Warn, 
	FName CallingContext)
{
	return NewObject<UCWeaponAsset>(InParent, InClass, InName, Flags);
}
