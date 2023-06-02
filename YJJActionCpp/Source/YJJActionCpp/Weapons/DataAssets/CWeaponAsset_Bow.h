#pragma once

#include "CoreMinimal.h"
#include "Weapons/CWeaponAsset.h"
#include "CWeaponAsset_Bow.generated.h"

class ACAttachment;
class UCEquipment;
class UCAct;

UCLASS()
class YJJACTIONCPP_API UCWeaponAsset_Bow : public UCWeaponAsset
{
	GENERATED_BODY()

public:
	UCWeaponAsset_Bow();

	virtual void BeginPlay(ACCommonCharacter* InOwner) override;

private:
	UPROPERTY(EditAnywhere)
		TSubclassOf<ACAttachment> SkillClass;

	enum { SkillMax = 10 };

	UPROPERTY(EditAnywhere)
		TArray<ACAttachment*> Skills;
};
