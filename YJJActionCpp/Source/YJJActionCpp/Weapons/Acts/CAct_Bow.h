#pragma once

#include "CoreMinimal.h"
#include "Weapons/CAct.h"
#include "CAct_Bow.generated.h"

class ACSkill_Arrow;

UCLASS()
class YJJACTIONCPP_API UCAct_Bow : public UCAct
{
	GENERATED_BODY()

public:
	virtual void Act() override;
	virtual void Begin_Act() override;
	virtual void End_Act() override;
};
