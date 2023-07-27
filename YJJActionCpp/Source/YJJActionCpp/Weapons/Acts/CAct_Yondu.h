#pragma once
#include "CoreMinimal.h"
#include "Weapons/CAct.h"
#include "CAct_Yondu.generated.h"

class ACSkillCollider_Yondu;

UCLASS(Blueprintable)
class YJJACTIONCPP_API UCAct_Yondu : public UCAct
{
	GENERATED_BODY()

public:
	UCAct_Yondu();

public:
	virtual void Tick(float InDeltaTime) override;

public:
	virtual void OnBeginEquip() override;
	virtual void OnUnequip() override;

	virtual void Act() override;

private:
	UPROPERTY(EditAnywhere, Category = "Spawn")
		FTransform DefaultTransform;

	UPROPERTY(EditAnywhere, Category = "Spawn")
		FVector LocationFactor = FVector(20, 50, 80);

	UPROPERTY(EditAnywhere, Category = "Spawn")
		TSubclassOf<ACSkillCollider_Yondu> YonduClass;

	UPROPERTY(VisibleAnywhere, Category = "Spawn")
		ACSkillCollider_Yondu* Yondu;
};
