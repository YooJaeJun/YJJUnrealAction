#pragma once

#include "CoreMinimal.h"
#include "Weapons/CEquipment.h"
#include "CEquipment_Warp.generated.h"

class UDecalComponent;
class UParticleSystemComponent;

UCLASS()
class YJJACTIONCPP_API UCEquipment_Warp : public UCEquipment
{
	GENERATED_BODY()

public:
	void BeginPlay(ACCommonCharacter* InOwner, const FEquipmentData& InData);

public:
	virtual void Equip_Implementation() override;
	virtual void Unequip_Implementation() override;

public:
	UPROPERTY(EditAnywhere)
		UDecalComponent* DecalComp;

	UPROPERTY(EditAnywhere)
		UParticleSystemComponent* ParticleComp;

private:
	TWeakObjectPtr<APlayerController> PlayerController;
};
