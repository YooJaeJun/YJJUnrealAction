#pragma once
#include "CoreMinimal.h"
#include "Weapons/CAct.h"
#include "CAct_Around.generated.h"

class ACSkillCollider_Rotate;

UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API UCAct_Around : public UCAct
{
	GENERATED_BODY()

public:
	virtual void Act() override;
	virtual void Begin_Act() override;

private:
	UPROPERTY(EditAnywhere, Category = "SpawnClass")
		TArray<TSubclassOf<ACSkillCollider_Rotate>> RotateClasses;
};