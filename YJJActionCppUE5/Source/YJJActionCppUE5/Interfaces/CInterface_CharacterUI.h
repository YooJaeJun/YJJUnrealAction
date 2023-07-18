#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CInterface_CharacterUI.generated.h"

UINTERFACE(MinimalAPI)
class UCInterface_CharacterUI : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPPUE5_API ICInterface_CharacterUI
{
	GENERATED_BODY()

public:
	virtual void InputAction_ActivateEquipMenu() {}
	virtual void InputAction_DeactivateEquipMenu() {}
};