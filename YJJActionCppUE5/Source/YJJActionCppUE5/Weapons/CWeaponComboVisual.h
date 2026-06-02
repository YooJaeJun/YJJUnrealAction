#pragma once

#include "CoreMinimal.h"
#include "Weapons/CWeaponCombo.h"
#include "CWeaponComboVisual.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;

/** Combo_Hammer / Combo_Sword / Combo_Dual EventGraph 장착·표시 분기를 하나로 묶는 모드. */
UENUM(BlueprintType)
enum class CEComboEquipVisualMode : uint8
{
	None UMETA(DisplayName = "None"),
	/** Sword — 장착 시 SkeletalMesh 를 Hand 소켓에 부착·표시. */
	SingleMeshOnEquip UMETA(DisplayName = "Single Mesh On Equip"),
	/** Hammer — BeginPlay 에 액터 Root 를 Hand 소켓에 부착, StaticMesh 는 장착 시만 표시. */
	SingleMeshRootOnBeginPlay UMETA(DisplayName = "Single Mesh Root On BeginPlay"),
	/** Dual — 좌/우 Capsule·Mesh 가시성 및 FallDownCapsule 부착. */
	DualWield UMETA(DisplayName = "Dual Wield"),
};

/**
 * 레거시 Combo_Hammer / Combo_Sword / Combo_Dual EventGraph — 메시·캡슐 장착/표시.
 * BP 자식과 UPROPERTY 동명 컴포넌트는 reparent 시 ICE — 런타임에 이름으로만 해석한다.
 */
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponComboVisual : public ACWeaponCombo
{
	GENERATED_BODY()

public:
	ACWeaponComboVisual();

	virtual void BeginPlay() override;

	virtual void Unequip_Implementation() override;

	virtual void Begin_Equip_Implementation(bool bMainOrSubWeapon) override;

	/** Dual — ProcessEvent(Skill) / 레거시 Skill 그래프. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Combo|EquipVisual")
	void Skill(int32 InSkillIndex);

	UFUNCTION(BlueprintCallable, Category = "Weapon|Combo|EquipVisual")
	void End_Skill();

	virtual void Host_Skill(int32 InSkillIndex) override;
	virtual void Host_EndSkill() override;

	/** Dual — 낙하 공격 구간 FallDownCapsule 부착(레거시 DoFallDownAttack 커스텀 함수). */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Combo|EquipVisual")
	void DoFallDownAttack();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Combo|EquipVisual")
	void End_DoFallDownAttack();

protected:
	virtual void Weapon_DoActionImpl(CEAttackType InAttackType, int32 InSkillIndex) override;

	virtual void ComboFork_End_FallDown() override;

	virtual void ComboFork_End_Skill() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|EquipVisual")
	CEComboEquipVisualMode EquipVisualMode = CEComboEquipVisualMode::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|EquipVisual")
	FName HandAttachSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|EquipVisual")
	FName HolsterAttachSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|EquipVisual|Dual")
	FName LeftHandSocketName = TEXT("Hand_Dual_L");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|EquipVisual|Dual")
	FName RightHandSocketName = TEXT("Hand_Dual_R");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|EquipVisual")
	FName SingleMeshComponentName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|EquipVisual|Dual")
	FName LeftMeshComponentName = TEXT("LeftMesh");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|EquipVisual|Dual")
	FName RightMeshComponentName = TEXT("RightMesh");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|EquipVisual|Dual")
	FName LeftCapsuleComponentName = TEXT("LeftCapsule");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|EquipVisual|Dual")
	FName RightCapsuleComponentName = TEXT("RightCapsule");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|EquipVisual|Dual")
	FName FallDownCapsuleComponentName = TEXT("FallDownCapsule");

private:
	void ComboEquip_ResolveComponents();
	USkeletalMeshComponent* ComboEquip_ResolveOwnerCharacterMesh() const;

	void ComboEquip_ApplyBeginPlayVisualState();
	void ComboEquip_ApplyUnequipVisualState();
	void ComboEquip_ApplyBeginEquipVisualState();

	void ComboEquip_AttachActorRootToCharacterHand();
	void ComboEquip_SetSingleMeshVisible(bool bVisible);
	void ComboEquip_AttachSingleMeshToHand();
	void ComboEquip_ApplyDualHiddenAndDetachedState();
	void ComboEquip_ApplyDualVisibleAndAttachedState();
	void ComboEquip_AttachFallDownCapsule();
	void ComboEquip_DetachFallDownCapsule();

	TObjectPtr<USceneComponent> ResolvedSingleMesh;
	TObjectPtr<USceneComponent> ResolvedLeftMesh;
	TObjectPtr<USceneComponent> ResolvedRightMesh;
	TObjectPtr<USceneComponent> ResolvedLeftCapsule;
	TObjectPtr<USceneComponent> ResolvedRightCapsule;
	TObjectPtr<USceneComponent> ResolvedFallDownCapsule;
};

// 레거시 `/Game/Weapons/Sword/Combo_Sword` — ctor 기본값만 Sword 용으로 설정.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponComboSword : public ACWeaponComboVisual
{
	GENERATED_BODY()

public:
	ACWeaponComboSword();
};

// 레거시 `/Game/Weapons/Hammer/Combo_Hammer`
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponComboHammer : public ACWeaponComboVisual
{
	GENERATED_BODY()

public:
	ACWeaponComboHammer();
};

// 레거시 `/Game/Weapons/Dual/Combo_Dual`
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponComboDual : public ACWeaponComboVisual
{
	GENERATED_BODY()

public:
	ACWeaponComboDual();
};
