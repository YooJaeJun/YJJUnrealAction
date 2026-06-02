#pragma once

#include "CoreMinimal.h"
#include "Commons/CEnums.h"
#include "Weapons/CWeaponStructures.h"
#include "Weapons/COwnerActor.h"
#include "Weapons/ICombatActionHost.h"
#include "Components/SceneComponent.h"
#include "Components/CMovementComponent.h"
#include "CWeapon.generated.h"

class ACPlayableCharacter;
class UCStateComponent;
class UDataTable;
class AController;
// 레거시 `/Game/Weapons/Weapon` 액터 C++ 매핑 — BPVar·루트 컴포넌트 + BeginPlay 에서 채우는 레퍼런스(Moving 등).
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeapon : public ACOwnerActor, public ICombatActionHost
{
	GENERATED_BODY()

public:
	ACWeapon();

	// ICombatActionHost — CWeaponComponent 가 ProcessEvent 대신 우선 호출.
	virtual void Host_DoAction(CEAttackType InAttackType, int32 InSkillIndex) override;
	virtual void Host_BeginDoAction(CEAttackType InAttackType) override;
	virtual void Host_EndDoAction(CEAttackType InAttackType) override;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

public:
	// 레거시 `/Game/Weapons/Weapon` 함수 그래프 — 액션 데이터 행 재생(Stamina 플레이어 전용)·이동 컴포넌트·이펙트·런치.
	UFUNCTION(BlueprintCallable, Category = "Weapon|Action")
	void PlayAction(const TArray<FDoActionData>& InDatas, int32 InIndex, const FVector& InHitPoint);

	// 레거시 Weapon_PlayAction 과 동일 카테고리 — 무기 액터 End_DoAction 그래프 + 활성 액트 정리까지 한 번에.
	UFUNCTION(BlueprintCallable, Category = "Weapon|Action", meta = (DisplayName = "End Do Action"))
	virtual void End_DoAction(CEAttackType InAttackType);

	UFUNCTION(BlueprintCallable, Category = "Weapon|Commons")
	void RestorePrevState();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Commons", meta = (ReturnDisplayName = "Enough Stamina"))
	bool ConsumeStamina(double InStamina);

	UFUNCTION(BlueprintCallable, Category = "Weapon|Commons", meta = (DisplayName = "Destroy"))
	void Destroy_FromWeaponBlueprint();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Actions")
	void DoAction(CEAttackType InAttackType, int32 InSkillIndex);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Actions")
	void Begin_DoAction(CEAttackType InAttackType);
	virtual void Begin_DoAction_Implementation(CEAttackType InAttackType);

	UFUNCTION(BlueprintCallable, Category = "Weapon|Dash")
	void CancelDashes();

	// ---------- 레거시 `/Game/Weapons/Weapon` — Sub Actions / Sub Weapon Actions ----------
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Sub Actions")
	void Pressed();
	virtual void Pressed_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Sub Actions")
	void Released();
	virtual void Released_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Sub Weapon Actions")
	void Hold_SubWeapon();
	virtual void Hold_SubWeapon_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Sub Weapon Actions")
	void Released_SubWeapon();
	virtual void Released_SubWeapon_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Sub Weapon Actions")
	void DoSubWeaponAction();
	virtual void DoSubWeaponAction_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Sub Weapon Actions")
	void Begin_DoSubWeaponAction();
	virtual void Begin_DoSubWeaponAction_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Sub Weapon Actions")
	void End_DoSubWeaponAction();
	virtual void End_DoSubWeaponAction_Implementation();

	// ---------- 레거시 `/Game/Weapons/Weapon` — Dash ----------
	UFUNCTION(BlueprintCallable, Category = "Weapon|Dash")
	void Dash();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Dash")
	void GroundDash();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Dash")
	void End_GroundDash();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Dash")
	void AirDash();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Dash")
	void End_AirDash();

	// ---------- 레거시 `/Game/Weapons/Weapon` Equips 카테고리 ----------
	UFUNCTION(BlueprintCallable, Category = "Weapon|Equip")
	void SetControllerInEquip();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Equip")
	void Equip();
	virtual void Equip_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Equip")
	void Unequip();
	virtual void Unequip_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Equip", meta = (CPP_Default_bMainOrSubWeapon = "true"))
	void Begin_Equip(bool bMainOrSubWeapon);
	virtual void Begin_Equip_Implementation(bool bMainOrSubWeapon);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Equip", meta = (CPP_Default_bMainOrSubWeapon = "true"))
	void End_Equip(bool bMainOrSubWeapon);
	virtual void End_Equip_Implementation(bool bMainOrSubWeapon);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Equip", meta = (CPP_Default_bMainOrSubWeapon = "true"))
	void Begin_Unequip(bool bMainOrSubWeapon);
	virtual void Begin_Unequip_Implementation(bool bMainOrSubWeapon);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Equip", meta = (CPP_Default_bMainOrSubWeapon = "true"))
	void End_Unequip(bool bMainOrSubWeapon);
	virtual void End_Unequip_Implementation(bool bMainOrSubWeapon);

private:
	void Weapon_PlayActionRunExecutionBranches(
		const FDoActionData& InRow,
		const FVector& InHitPoint,
		ACPlayableCharacter* OptionalPlayer);

protected:
	// 레거시 EventGraph — ReceiveBeginPlay 복합 노드 순서(Set Character→Set Component→Load*).
	void Weapon_SetCharacterAndComponentsFromBlueprint();
	void Weapon_ClearRuntimeTableCaches();
	void Weapon_LoadEquipDatasFromTable();
	void Weapon_LoadDoActionDatasFromTables();
	void Weapon_LoadHitDatasFromTable();

	static void Weapon_AppendDoActionRowByAttackType(ACWeapon* InWeapon, const FDoActionData& InRow);

	/** DoMovingActionDataTable 분기만 — 레거시 Dash / AirDash 만 DoMovingActionDatas 로 적재한다. */
	static void Weapon_AppendMovingDoActionRowIfDashTypes(ACWeapon* InWeapon, const FDoActionData& InRow);

	static void Weapon_AppendHitRowByAttackType(ACWeapon* InWeapon, const FHitData& InRow);

	/** 레거시 Equip 표 분기 — UseControlRotation / CanMove 에 따라 이동 컴포넌트만 갱신한다. */
	void Weapon_ApplyEquipRowFacingAndMovement(const FEquipData& Row);

	/** Weapon.State 또는 캐릭터에서 UCStateComponent 해석 — 대시·낙하 분기에 공통 사용. */
	UCStateComponent* Weapon_ResolveStateComponent() const;

	/** State 또는 CharacterMovement 로 공중(낙하·비행) 여부 — Dash 의 AirDash 분기. */
	bool Weapon_ShouldAirDashFromMovementState() const;

	/** 레거시 Weapon::DoAction 본체 — 콤보 무기(ACWeaponCombo) 에서 타입 디스패치를 덮어쓴다. */
	virtual void Weapon_DoActionImpl(CEAttackType InAttackType, int32 InSkillIndex);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	// 레거시 BP 표시 이름 "Moving". BeginPlay 에서 장착 캐릭터의 UCMovementComponent 를 가리킨다(무기 액터에 별도 이동 서브컴포넌트를 두지 않음).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
	TObjectPtr<UCMovementComponent> Moving;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Settings")
	TObjectPtr<AController> Controller;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Settings")
	CEAttackType PrevAttackType = CEAttackType::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|References")
	TObjectPtr<UObject> State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TObjectPtr<UDataTable> EquipDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	FEquipData EquipData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	FEquipData UnequipData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TObjectPtr<UDataTable> DoActionDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FDoActionData> DoActionDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FDoActionData> DoAirComboDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FDoActionData> DoFlyingAttackDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FDoActionData> DoDownAttackDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FDoActionData> DoSwayingAttackDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FDoActionData> DoFallDownAttackDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FDoActionData> SkillDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FDoActionData> DoRidingAttackDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TObjectPtr<UDataTable> DoMovingActionDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FDoActionData> DoMovingActionDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TObjectPtr<UDataTable> HitDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FHitData> HitCommonDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FHitData> HitAirComboDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FHitData> HitFlyingAttackDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FHitData> HitDownAttackDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FHitData> HitSwayingAttackDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FHitData> HitFallDownAttackDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FHitData> HitSkillDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DataTables")
	TArray<FHitData> HitRidingAttackDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Equip")
	bool IsEquipping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Equip")
	bool InEquip = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Equip")
	bool IsUnequipping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Action")
	bool InAction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Action")
	bool InSubWeaponAction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Action")
	bool InGroundDashing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Action")
	bool InAirDashing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Action")
	int32 GroundDashIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Action")
	int32 AirDashIndex = 1;
};
