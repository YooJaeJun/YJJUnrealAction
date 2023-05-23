#pragma once

#include "CoreMinimal.h"
#include "Character/CCommonCharacter.h"
#include "Character/CInterface_CharacterBody.h"
#include "Weapons/CWeaponStructures.h"
#include "CEnemy.generated.h"

class AController;
class UCWeaponComponent;
struct FDamageEvent;

UCLASS(Abstract)
class YJJACTIONCPP_API ACEnemy :
	public ACCommonCharacter
{
	GENERATED_BODY()
};
