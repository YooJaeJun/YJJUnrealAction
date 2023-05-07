#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CWeaponComponent.generated.h"

class ACCommonCharacter;
class ACWeapon;
class ACAttachment;
class UCEquipment;
class UCAct;
class UCWeaponAsset;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Unarmed,
	Sword,
	Fist,
	Hammer,
	Bow,
	Dual,
	Guard,
	Max
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponTypeChanged, const EWeaponType, InPrevType, const EWeaponType, InNewType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCWeaponComponent();

protected:
	virtual void BeginPlay() override;

public:
	void Begin_Equip();
	void End_Equip();
	void Begin_Act();
	void End_Act();
	void Act();

public:
	void SetMode(EWeaponType InType);

private:
	void ChangeType(EWeaponType InType);

public:
	ACAttachment* GetAttachment() const;
	UCEquipment* GetEquipment() const;
	UCAct* GetAct() const;

	bool IsIdleMode() const;

	void SetUnarmedMode();
	void SetSwordMode();
	void SetFistMode();
	void SetHammerMode();
	void SetBowMode();
	void SetDualMode();

public:
	FORCEINLINE bool IsUnarmedMode() const { return Type == EWeaponType::Unarmed; }
	FORCEINLINE bool IsSwordMode() const { return Type == EWeaponType::Sword; }
	FORCEINLINE bool IsFistMode() const { return Type == EWeaponType::Fist; }
	FORCEINLINE bool IsBowMode() const { return Type == EWeaponType::Bow; }
	FORCEINLINE bool IsDualMode() const { return Type == EWeaponType::Dual; }

private:
	UPROPERTY(EditAnywhere, Category = "DataAsset")
		UCWeaponAsset* DataAssets[static_cast<uint8>(EWeaponType::Max)];

	UPROPERTY(EditAnyWhere, Category = "Settings")
		EWeaponType Type = EWeaponType::Unarmed;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		EWeaponType PrevType = EWeaponType::Max;

public:
	FWeaponTypeChanged OnWeaponTypeChanged;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TArray<TSharedPtr<ACWeapon>> Weapons;
	TSharedPtr<ACWeapon> Weapon;
};
