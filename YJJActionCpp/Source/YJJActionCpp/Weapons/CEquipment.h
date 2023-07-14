#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapons/CWeaponStructures.h"
#include "CEquipment.generated.h"

class ACCommonCharacter;
class UCMovementComponent;
class UCStateComponent;
class UCCamComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentBeginEquip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentUnequip);

UCLASS()
class YJJACTIONCPP_API UCEquipment : public UObject
{
	GENERATED_BODY()

public:
	virtual void BeginPlay(TWeakObjectPtr<ACCommonCharacter> InOwner, const FEquipmentData& InData);

public:
	UFUNCTION(BlueprintNativeEvent)
		void Equip();
	virtual void Equip_Implementation();

	UFUNCTION(BlueprintNativeEvent)
		void Begin_Equip();
	virtual void Begin_Equip_Implementation();

	UFUNCTION(BlueprintNativeEvent)
		void End_Equip();
	virtual void End_Equip_Implementation();

	UFUNCTION(BlueprintNativeEvent)
		void Unequip();
	virtual void Unequip_Implementation();

public:
	FORCEINLINE TSharedPtr<bool> GetBeginEquip() const { return  MakeShared<bool>(bBeginEquip); }
	FORCEINLINE TSharedPtr<bool> GetEquipped() const { return MakeShared<bool>(bEquipped); }

public:
	FEquipmentBeginEquip OnEquipmentBeginEquip;
	FEquipmentUnequip OnEquipmentUnequip;

protected:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	FEquipmentData Data;

private:
	TWeakObjectPtr<UCMovementComponent> MovementComp;
	TWeakObjectPtr<UCStateComponent> StateComp;
	TWeakObjectPtr<UCCamComponent> CamComp;

private:
	bool bBeginEquip;
	bool bEquipped;
};