#include "Weapons/DataAssets/CWeaponAsset_Bow.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Utilities/CHelpers.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CEquipment.h"
#include "Weapons/CAct.h"

UCWeaponAsset_Bow::UCWeaponAsset_Bow()
{
	SkillClass = ACAttachment::StaticClass();
}

void UCWeaponAsset_Bow::BeginPlay(TWeakObjectPtr<ACCommonCharacter> InOwner)
{
	Super::BeginPlay(InOwner);
}
