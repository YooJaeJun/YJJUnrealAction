#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapons/CWeaponStructures.h"
#include "CEquipment.generated.h"

class ACCommonCharacter;
class UCMovementComponent;
class UCStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentBeginEquip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentUnequip);

UCLASS()
class YJJACTIONCPP_API UCEquipment : public UObject
{
	GENERATED_BODY()

public:
	void BeginPlay(ACCommonCharacter* InOwner, const FEquipmentData& InData);

public:
	UFUNCTION(BlueprintNativeEvent)
		void Equip();
	void Equip_Implementation();

	UFUNCTION(BlueprintNativeEvent)
		void Begin_Equip();
	void Begin_Equip_Implementation();

	UFUNCTION(BlueprintNativeEvent)
		void End_Equip();
	void End_Equip_Implementation();

	UFUNCTION(BlueprintNativeEvent)
		void Unequip();
	void Unequip_Implementation();

public:
	FEquipmentBeginEquip OnEquipmentBeginEquip;
	FEquipmentUnequip OnEquipmentUnequip;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	FEquipmentData Data;

private:
	TWeakObjectPtr<UCMovementComponent> Movement;
	TWeakObjectPtr<UCStateComponent> State;

private:
	bool bBeginEquip;
	bool bEquipped;
};
