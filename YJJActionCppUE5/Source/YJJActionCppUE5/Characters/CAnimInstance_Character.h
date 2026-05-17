#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Characters/CCharacterFeetData.h"
#include "Commons/CEnums.h"
#include "Components/ActorComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CWeaponComponent.h"
#include "CAnimInstance_Character.generated.h"

class ACCommonCharacter;
class UCMovementComponent;
class UCFlyComponent;
class UCMagicComponent;

UCLASS(Abstract)
class YJJACTIONCPPUE5_API UCAnimInstance_Character : public UAnimInstance
{
	GENERATED_BODY()

	// ACAnimalAnimDynamicInfo::Tick_Info 가 NativeUpdate 이후 스냅샷을 복사할 수 있게 한다.
	friend class ACAnimalAnimDynamicInfo;

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UFUNCTION()
	void OnStateTypeChanged(const CEStateType InPrevType, const CEStateType InNewType);

	UFUNCTION()
	void OnWeaponTypeChanged(
		const CEWeaponType InPrevMainType,
		const CEWeaponType InNewMainType,
		const CEWeaponType InPrevSubType,
		const CEWeaponType InNewSubType);

	UFUNCTION()
	void OnMagicTypeChanged(CEMagicType InType, CEMagicType InPrevType);

	// --- ABP_Character 의 BPVar — AnimBP 리패어런트 시 핀 이름 호환. ---

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default", meta = (DisplayName = "Weapon Component"))
	TObjectPtr<UCWeaponComponent> WeaponComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default", meta = (DisplayName = "Magic Component"))
	TObjectPtr<UCMagicComponent> MagicComponent;

	// 이름 "Character" 는 UCharacter·ABP 변수 검증과 겹치기 쉬워 핀 깨짐이 난다 — AnimBP 에서는 Owning Character 로만 접근한다.
	UPROPERTY(
		BlueprintReadWrite,
		EditAnywhere,
		Category = "세팅",
		meta = (DisplayName = "Owning Character"))
	TObjectPtr<ACCommonCharacter> OwningCharacter;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default", meta = (DisplayName = "State Component"))
	TObjectPtr<UCStateComponent> StateComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	float Speed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "세팅", meta = (DisplayName = "Direction"))
	double Direction = 0.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "세팅", meta = (DisplayName = "Pitch"))
	double Pitch = 0.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "세팅", meta = (DisplayName = "Neck Yaw"))
	double Neck_Yaw = 0.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "세팅", meta = (DisplayName = "Prev Rotation"))
	FRotator PrevRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States", meta = (DisplayName = "State"))
	CEStateType State = CEStateType::Idle;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States", meta = (DisplayName = "Prev State"))
	CEStateType PrevState = CEStateType::Idle;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States", meta = (DisplayName = "Falling"))
	bool Falling = false;

	/** CABP 레거시 bFalling — Anim Graph Set 노드 호환용(Falling 과 동일 값 동기화). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "States", meta = (DisplayName = "bFalling Legacy"))
	bool bFalling = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States", meta = (DisplayName = "Hitting"))
	bool Hitting = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapons", meta = (DisplayName = "Main Weapon Type"))
	CEWeaponType MainWeaponType = CEWeaponType::Unarmed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapons", meta = (DisplayName = "Sub Weapon Type"))
	CEWeaponType SubWeaponType = CEWeaponType::Unarmed;

	/** CABP 레거시 핀 "Weapon Type" — NativeUpdate 에서 마법 슬롯·실물 무기별 블렌드용으로 채운다. */
	UPROPERTY(
		BlueprintReadWrite,
		EditAnywhere,
		Category = "Weapons",
		meta = (DisplayName = "Weapon Type Legacy Pin"))
	CEWeaponType WeaponType = CEWeaponType::Unarmed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapons", meta = (DisplayName = "Magic Type"))
	CEMagicType MagicType = CEMagicType::Unarmed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Bows", meta = (DisplayName = "Bow Aiming"))
	bool Bow_Aiming = false;

	/** CABP 레거시 bBowAiming — Bow_Aiming 과 동일. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bows", meta = (DisplayName = "bBowAiming Legacy"))
	bool bBowAiming = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Feets", meta = (DisplayName = "Feet"))
	bool Feet = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Feets", meta = (DisplayName = "Feet Data"))
	FFeetData FeetData;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Riding", meta = (DisplayName = "Riding"))
	bool bRiding = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Riding", meta = (DisplayName = "Riding Animal"))
	TObjectPtr<AActor> RidingAnimal;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Riding", meta = (DisplayName = "Riding Animal Falling"))
	bool RidingAnimalFalling = false;

	/** CABP 레거시 bRidingFalling — RidingAnimalFalling 과 동일. */
	UPROPERTY(
		BlueprintReadWrite,
		EditAnywhere,
		Category = "Riding",
		meta = (DisplayName = "bRidingFalling Legacy"))
	bool bRidingFalling = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Riding", meta = (DisplayName = "Riding Speed"))
	double RidingSpeed = 0.0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Riding", meta = (DisplayName = "Controller Normalized Direction"))
	double ControllerNormalizedDirection = 0.0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Riding", meta = (DisplayName = "Controller Direction"))
	double ControllerDirection = 0.0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Riding", meta = (DisplayName = "Real Riding"))
	bool bRealRiding = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Riding", meta = (DisplayName = "Leg IKAlpha"))
	double LegIKAlpha = 1.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	float Yaw;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	float Look;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	float Forward;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	float Side;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "IK")
	bool bFootIK = true;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "IK")
	bool bRidingIK;

protected:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCStateComponent> StateComp;
	TWeakObjectPtr<UCMovementComponent> MovementComp;
	TWeakObjectPtr<UCFlyComponent> FlyComp;
};
