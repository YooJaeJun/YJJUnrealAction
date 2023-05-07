#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CInterface_CharacterAnim.generated.h"

UINTERFACE(MinimalAPI)
class UCInterface_CharacterAnim : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPP_API ICInterface_CharacterAnim
{
	GENERATED_BODY()

public:
	virtual void End_Avoid() {}
	virtual void End_Hit() {}
};
