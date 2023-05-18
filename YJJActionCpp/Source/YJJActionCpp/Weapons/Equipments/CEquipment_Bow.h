#pragma once

#include "CoreMinimal.h"
#include "Weapons/CEquipment.h"
#include "CEquipment_Bow.generated.h"

UCLASS()
class YJJACTIONCPP_API UCEquipment_Bow : public UCEquipment
{
	GENERATED_BODY()

public:
	virtual void End_Equip_Implementation() override;
	virtual void Unequip_Implementation() override;
};
