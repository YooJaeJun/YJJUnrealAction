#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CInterface_Interactable.generated.h"

class ACCommonCharacter;

UINTERFACE(MinimalAPI)
class UCInterface_Interactable : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPPUE5_API ICInterface_Interactable
{
	GENERATED_BODY()

public:
	virtual void Interact(TObjectPtr<ACCommonCharacter> InteractingActor) {}
};
