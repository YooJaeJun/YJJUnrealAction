#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CInterface_Rideable.generated.h"

class ACCommonCharacter;

UINTERFACE(MinimalAPI)
class UCInterface_Rideable : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPP_API ICInterface_Rideable
{
	GENERATED_BODY()

public:
	virtual void RegisterInteractableCharacter(ACCommonCharacter* InteractableCharacter) {}
};
