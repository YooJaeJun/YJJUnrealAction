#pragma once
#include "CoreMinimal.h"
#include "Weapons/CAct.h"
#include "CAct_Bomb.generated.h"

class ACSkillCollider_Bomb;

UCLASS(Blueprintable)
class YJJACTIONCPP_API UCAct_Bomb : public UCAct
{
	GENERATED_BODY()

public:
	virtual void Act() override;
	virtual void Begin_Act() override;

private:
	UPROPERTY(EditAnywhere, Category = "SpawnClass")
		TSubclassOf<ACSkillCollider_Bomb> BombClass;
};
