#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CInterface_CharacterState.generated.h"

UINTERFACE(MinimalAPI)
class UCInterface_CharacterState : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPP_API ICInterface_CharacterState
{
	GENERATED_BODY()

public:
	virtual void Avoid() {}
	virtual void Rise() {}
	virtual void Land() {}
	virtual void Hit() {}
	virtual void Dead() {}
	virtual void End_Avoid() {}
	virtual void End_Rise() {}
	virtual void End_Hit() {}
	virtual void End_Dead() {}
};
