#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CInterface_CharacterUI.generated.h"

UINTERFACE(MinimalAPI)
class UCInterface_CharacterUI : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPP_API ICInterface_CharacterUI
{
	GENERATED_BODY()

public:
	virtual void OnMenu() {}
	virtual void ActivateEquipMenu() {}
	virtual void DeactivateEquipMenu() {}
	virtual void HoveredEquipMenu() {}
	virtual void UnhoveredEquipMenu() {}
	virtual void ClickedEquipMenu() {}
};
