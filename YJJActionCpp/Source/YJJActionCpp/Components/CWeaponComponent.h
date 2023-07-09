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
class UCStateComponent;
class UCMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponTypeChanged, 
	const CEWeaponType, InPrevType, 
	const CEWeaponType, InNewType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCWeaponComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InputAction_Act();
	void InputAction_Skill_Pressed();
	void InputAction_Skill_Released();

	void SetModeFromZeroIndex();
	void SetMode(CEWeaponType InType);
	void CancelAct();

private:
	void ChangeType(CEWeaponType InType);

public:
	TWeakObjectPtr<UCWeaponAsset> GetWeaponAsset();
	ACAttachment* GetAttachment();
	UCEquipment* GetEquipment();
	UCAct* GetAct();
	UCSkill* GetSkill();

	FORCEINLINE constexpr CEWeaponType GetType() const { return Type; }
	FORCEINLINE constexpr CEWeaponType GetPrevType() const { return PrevType; }

	bool IsIdleStateMode();

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
	FORCEINLINE constexpr bool IsUnarmedMode() const { return Type == CEWeaponType::Unarmed; }
	FORCEINLINE constexpr bool IsFistMode() const { return Type == CEWeaponType::Fist; }
	FORCEINLINE constexpr bool IsSwordMode() const { return Type == CEWeaponType::Sword; }
	FORCEINLINE constexpr bool IsHammerMode() const { return Type == CEWeaponType::Hammer; }
	FORCEINLINE constexpr bool IsDualMode() const { return Type == CEWeaponType::Dual; }
	FORCEINLINE constexpr bool IsBowMode() const { return Type == CEWeaponType::Bow; }
	FORCEINLINE constexpr bool IsWarpMode() const { return Type == CEWeaponType::Warp; }
	FORCEINLINE constexpr bool IsAroundMode() const { return Type == CEWeaponType::Around; }
	FORCEINLINE constexpr bool IsFireballMode() const { return Type == CEWeaponType::Fireball; }
	FORCEINLINE constexpr bool IsBombMode() const { return Type == CEWeaponType::Bomb; }
	FORCEINLINE constexpr bool IsYonduMode() const { return Type == CEWeaponType::Yondu; }

private:
	// DataAsset을 객체마다 만들지 않게 하기 위해 Copy를 따로 만듦
	UPROPERTY(EditAnywhere, Category = "DataAsset")
		TArray<UCWeaponAsset*> DataAssets;

	UPROPERTY(VisibleAnywhere, Category = "DataAsset")
		TMap<CEWeaponType, UCWeaponAsset*> DataAssetMap;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		CEWeaponType Type = CEWeaponType::Unarmed;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		CEWeaponType PrevType = CEWeaponType::Max;

public:
	FWeaponTypeChanged OnWeaponTypeChanged;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCStateComponent> StateComp;
	TWeakObjectPtr<UCMovementComponent> MovementComp;
};
