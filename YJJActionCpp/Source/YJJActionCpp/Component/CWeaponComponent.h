#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CWeaponComponent.generated.h"

class ACCommonCharacter;
class ACWeapon;

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponTypeChanged, EWeaponType, InPrevType, EWeaponType, InNewType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnyWhere, Category = "Settings")
		TArray<TSubclassOf<ACWeapon>> WeaponClasses;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		EWeaponType Type = EWeaponType::Max;

public:
	FORCEINLINE bool IsUnarmedMode() const { return Type == EWeaponType::Unarmed; }
	FORCEINLINE bool IsSwordMode() const { return Type == EWeaponType::Sword; }
	FORCEINLINE bool IsFistMode() const { return Type == EWeaponType::Fist; }
	FORCEINLINE bool IsBowMode() const { return Type == EWeaponType::Bow; }
	FORCEINLINE bool IsDualMode() const { return Type == EWeaponType::Dual; }

public:	
	UCWeaponComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetUnarmedMode();
	void SetSwordMode();
	void SetFistMode();
	void SetHammerMode();
	void SetBowMode();
	void SetDualMode();

	void Begin_Equip();
	void End_Equip();
	void Begin_Act();
	void End_Act();

public:
	FWeaponTypeChanged OnWeaponTypeChanged;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TArray<TSharedPtr<ACWeapon>> Weapons;
	TSharedPtr<ACWeapon> Weapon;
};
