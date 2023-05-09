#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CWeaponComponent.generated.h"

class ACCommonCharacter;
class ACAttachment;
class UCEquipment;
class UCAct;
class UCWeaponAsset;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Unarmed,
	Fist,
	Sword,
	Hammer,
	Dual,
	Bow,
	Guard,
	Around,
	Fireball,
	Bomb,
	Yondu,
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
	void Act();
	void SetMode(EWeaponType InType);

private:
	void ChangeType(EWeaponType InType);

public:
	ACAttachment* GetAttachment() const;
	UCEquipment* GetEquipment() const;
	UCAct* GetAct() const;

	bool IsIdleStateMode() const;

	void SetUnarmedMode();
	void SetSwordMode();
	void SetFistMode();
	void SetHammerMode();
	void SetDualMode();
	void SetBowMode();
	void SetAroundMode();
	void SetFireballMode();
	void SetBombMode();
	void SetYonduMode();

public:
	FORCEINLINE bool IsUnarmedMode() const { return Type == EWeaponType::Unarmed; }
	FORCEINLINE bool IsSwordMode() const { return Type == EWeaponType::Sword; }
	FORCEINLINE bool IsFistMode() const { return Type == EWeaponType::Fist; }
	FORCEINLINE bool IsDualMode() const { return Type == EWeaponType::Dual; }
	FORCEINLINE bool IsBowMode() const { return Type == EWeaponType::Bow; }
	FORCEINLINE bool IsAroundMode() const { return Type == EWeaponType::Around; }
	FORCEINLINE bool IsFireballMode() const { return Type == EWeaponType::Fireball; }
	FORCEINLINE bool IsBombMode() const { return Type == EWeaponType::Bomb; }
	FORCEINLINE bool IsYonduMode() const { return Type == EWeaponType::Yondu; }

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
};
