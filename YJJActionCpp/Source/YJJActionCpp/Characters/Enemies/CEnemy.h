#pragma once

#include "CoreMinimal.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/CInterface_CharacterBody.h"
#include "Weapons/CWeaponStructures.h"
#include "CEnemy.generated.h"

UCLASS(Abstract)
class YJJACTIONCPP_API ACEnemy :
	public ACCommonCharacter
{
	GENERATED_BODY()
};
