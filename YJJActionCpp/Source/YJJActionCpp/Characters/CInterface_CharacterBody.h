#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CInterface_CharacterBody.generated.h"

class ACCommonCharacter;

UINTERFACE(MinimalAPI)
class UCInterface_CharacterBody : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPP_API ICInterface_CharacterBody
{
	GENERATED_BODY()

public:
	void Create_DynamicMaterial(ACCommonCharacter* InCharacter);
	void ChangeColor(ACCommonCharacter* InCharacter, FLinearColor InColor);
};
