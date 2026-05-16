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
class YJJACTIONCPPUE5_API UCEquipment : public UObject
{
	GENERATED_BODY()

public:
	virtual void BeginPlay(TWeakObjectPtr<ACCommonCharacter> InOwner, const FEquipData& InData);

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

	// 레거시 애니 ANS_Unequip 노티 상태 — 해제 몽타주 시작/종료를 Equip 의 Begin_/End_Equip 과 같은 축으로 둔다.
	UFUNCTION(BlueprintNativeEvent)
	void Begin_Unequip();
	virtual void Begin_Unequip_Implementation();

	UFUNCTION(BlueprintNativeEvent)
	void End_Unequip();
	virtual void End_Unequip_Implementation();

public:
	FORCEINLINE TSharedPtr<bool> GetBeginEquip() const { return  MakeShared<bool>(bBeginEquip); }
	FORCEINLINE TSharedPtr<bool> GetEquipped() const { return MakeShared<bool>(bEquipped); }

public:
	FEquipmentBeginEquip OnEquipmentBeginEquip;
	FEquipmentUnequip OnEquipmentUnequip;

protected:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	FEquipData Data;

private:
	TWeakObjectPtr<UCMovementComponent> MovementComp;
	TWeakObjectPtr<UCStateComponent> StateComp;
	TWeakObjectPtr<UCCamComponent> CamComp;

private:
	bool bBeginEquip;
	bool bEquipped;
};