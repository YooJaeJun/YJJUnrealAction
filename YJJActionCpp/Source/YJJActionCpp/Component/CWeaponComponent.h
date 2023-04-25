#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CWeaponComponent.generated.h"

class ACPlayer;
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

public:	
	UCWeaponComponent();

private:
	UPROPERTY(EditAnyWhere, Category = "Settings")
		TArray<TSubclassOf<ACWeapon>> WeaponClasses;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	FORCEINLINE bool IsUnarmed() { return Type == EWeaponType::Unarmed; }
	FORCEINLINE bool IsSword() { return Type == EWeaponType::Sword; }
	FORCEINLINE bool IsFist() { return Type == EWeaponType::Fist; }
	FORCEINLINE bool IsBow() { return Type == EWeaponType::Bow; }
	FORCEINLINE bool IsDual() { return Type == EWeaponType::Dual; }

	void SetUnarmed();
	void SetSword();
	void SetFist();
	void SetHammer();
	void SetBow();
	void SetDual();

	void Begin_Equip();
	void End_Equip();
	void Begin_Act();
	void End_Act();

public:
	FWeaponTypeChanged OnWeaponTypeChanged;

private:
	EWeaponType Type = EWeaponType::Max;

private:
	TWeakObjectPtr<ACPlayer> Owner;
	TArray<TSharedPtr<ACWeapon>> Weapons;
};
