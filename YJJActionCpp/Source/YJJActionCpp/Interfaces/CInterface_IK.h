#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CInterface_IK.generated.h"

UINTERFACE(MinimalAPI)
class UCInterface_IK : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPP_API ICInterface_IK
{
	GENERATED_BODY()

public:
	void SetLegIKAlpha(const float InAlpha);
	float GetLegIKAlpha() const;

private:
	float LegIKAlpha = 0.3f;
};
