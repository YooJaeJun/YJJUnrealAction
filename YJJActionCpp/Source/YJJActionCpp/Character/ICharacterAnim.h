#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ICharacterAnim.generated.h"

UINTERFACE(MinimalAPI)
class UICharacterAnim : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPP_API IICharacterAnim
{
	GENERATED_BODY()

public:
	virtual void End_BackStep() {}
};
