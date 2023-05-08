#pragma once

#include "CoreMinimal.h"
#include "Weapons/CAct.h"
#include "CAct_Combo.generated.h"

UCLASS()
class YJJACTIONCPP_API UCAct_Combo : public UCAct
{
	GENERATED_BODY()

public:
	FORCEINLINE void EnableCombo() { bEnable = true; }
	FORCEINLINE void DisableCombo() { bEnable = false; }

public:
	virtual void Act() override;
	virtual void Begin_Act() override;
	virtual void End_Act() override;

private:
	int32 Index;

	bool bEnable;
	bool bExist;
};
