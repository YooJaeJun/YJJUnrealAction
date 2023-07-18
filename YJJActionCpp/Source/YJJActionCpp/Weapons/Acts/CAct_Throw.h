#pragma once
#include "CoreMinimal.h"
#include "Weapons/CAct.h"
#include "CAct_Throw.generated.h"

class ACSkillCollider_Throw;

UCLASS(Blueprintable)
class YJJACTIONCPP_API UCAct_Throw : public UCAct
{
	GENERATED_BODY()

public:
	virtual void Act() override;
	virtual void Begin_Act() override;

private:
	UPROPERTY(EditAnywhere, Category = "SpawnClass")
		TArray<TSubclassOf<ACSkillCollider_Throw>> ThrowClasses;
};
