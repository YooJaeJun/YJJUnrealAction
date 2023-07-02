#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/CWeaponAsset.h"
#include "Commons/CEnums.h"
#include "CWeaponComponent.generated.h"

class ACCommonCharacter;
class ACAttachment;
class UCEquipment;
class UCAct;

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
	void InputAction_Act();
	void SetModeFromZeroIndex();
	void SetMode(EWeaponType InType);
	int32 FindType(const EWeaponType InType);
	void CancelAct();

private:
	void ChangeType(EWeaponType InType);

public:
	ACAttachment* GetAttachment();
	UCEquipment* GetEquipment();
	UCAct* GetAct();

	FORCEINLINE EWeaponType GetType() const { return Type; }
	FORCEINLINE EWeaponType GetPrevType() const { return PrevType; }

	bool IsIdleStateMode() const;

	void SetUnarmedMode();
	void SetSwordMode();
	void SetFistMode();
	void SetHammerMode();
	void SetDualMode();
	void SetBowMode();
	void SetWarpMode();
	void SetAroundMode();
	void SetFireballMode();
	void SetBombMode();
	void SetYonduMode();

public:
	FORCEINLINE bool IsUnarmedMode() const { return Type == EWeaponType::Unarmed; }
	FORCEINLINE bool IsFistMode() const { return Type == EWeaponType::Fist; }
	FORCEINLINE bool IsSwordMode() const { return Type == EWeaponType::Sword; }
	FORCEINLINE bool IsHammerMode() const { return Type == EWeaponType::Hammer; }
	FORCEINLINE bool IsDualMode() const { return Type == EWeaponType::Dual; }
	FORCEINLINE bool IsBowMode() const { return Type == EWeaponType::Bow; }
	FORCEINLINE bool IsWarpMode() const { return Type == EWeaponType::Warp; }
	FORCEINLINE bool IsAroundMode() const { return Type == EWeaponType::Around; }
	FORCEINLINE bool IsFireballMode() const { return Type == EWeaponType::Fireball; }
	FORCEINLINE bool IsBombMode() const { return Type == EWeaponType::Bomb; }
	FORCEINLINE bool IsYonduMode() const { return Type == EWeaponType::Yondu; }

private:
	// DataAsset을 객체마다 만들지 않게 하기 위해 Copy를 따로 만듦
	UPROPERTY(EditAnywhere, Category = "DataAsset")
		TArray<UCWeaponAsset*> DataAssets;

	UPROPERTY(VisibleAnywhere, Category = "DataAsset")
		TArray<UCWeaponAsset*> DataAssetsCopied;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		EWeaponType Type = EWeaponType::Unarmed;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		EWeaponType PrevType = EWeaponType::Max;

public:
	FWeaponTypeChanged OnWeaponTypeChanged;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
};
