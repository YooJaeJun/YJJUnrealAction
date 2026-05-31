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
class UCSkill;
class UCStateComponent;
class UCMovementComponent;

// 블루프린트 BPWeaponComponent 의 OnWeaponTypeChanged 와 동일한 시그널 순서(InPrev/Main, InNew/Main, InPrev/Sub, InNew/Sub).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FWeaponTypeChanged,
	const CEWeaponType, InPrevMainType,
	const CEWeaponType, InNewMainType,
	const CEWeaponType, InPrevSubType,
	const CEWeaponType, InNewSubType);

// 마법 변경 — BP 순서(InType 현재 값, InPrevType 이전 값)와 동일. MagicComponent 가 있으면 그쪽 디스패치가 대신 처리한다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMagicTypeChangedMulticast,
	CEMagicType, InType,
	CEMagicType, InPrevType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPPUE5_API UCWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

	// BP 기본 어레이 길이(9). 설계 시 슬롯 수를 바꾸면 디폴트 서브오브젝트·세이브와 충돌할 수 있다.
	static constexpr int32 WeaponEquipmentSlotCount = 9;

public:
	UCWeaponComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InputAction_Act();
	void InputAction_SubAct_Pressed();
	void InputAction_SubAct_Released();
	void InputAction_Skill_1_Pressed();
	void InputAction_Skill_1_Released();
	void InputAction_Skill_2_Pressed();
	void InputAction_Skill_2_Released();
	void InputAction_Skill_3_Pressed();
	void InputAction_Skill_3_Released();

	void SetModeFromZeroIndex();
	void SetMode(CEWeaponType InType);
	void SetMagicMode(CEMagicType InMagicType);
	void CancelAct();

	/** 레거시 BP WeaponComponent::End Do Action 애니 규약 — 활성 물리 Act 의 End_Act (마법은 UCMagicComponent::End_DoAction 사용). */
	UFUNCTION(BlueprintCallable, Category = "Weapons|Skill", DisplayName = "End Do Action")
	void End_DoAction(CEAttackType InAttackType);

	/** 레거시 AN_Begin_Action — Magic(IsUnarmed) 분기 후 MainWeapon Begin_DoAction. */
	UFUNCTION(BlueprintCallable, Category = "Weapons|Skill", DisplayName = "Begin Do Action")
	void Begin_DoAction(CEAttackType InAttackType);

	/** 레거시 AN_Begin/End_AirCombo 등 — 콤보 AttackType 을 MainWeapon 으로 전달. */
	UFUNCTION(BlueprintCallable, Category = "Weapons|Skill|Combo", DisplayName = "Begin Do Air Combo")
	void Begin_DoAirCombo();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Skill|Combo", DisplayName = "End Do Air Combo")
	void End_DoAirCombo();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Skill|Combo", DisplayName = "Begin Do Flying Attack")
	void Begin_DoFlyingAttack();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Skill|Combo", DisplayName = "End Do Flying Attack")
	void End_DoFlyingAttack();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Skill|Combo", DisplayName = "End Do Down Attack")
	void End_DoDownAttack();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Skill|Combo", DisplayName = "End Dash Attack")
	void End_DashAttack();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Skill|Combo", DisplayName = "End Fall Down")
	void End_FallDown();

	/** AN_ConsumeStamina — MainWeapon 스태미나 소모, 부족 시 End_DoAction + 몽타주 중단. */
	bool TryNotifyConsumeStamina(double InStamina);

	/** AN_BoxCollision — MainWeapon 이 콤보 무기일 때 Box 콜리전만 켠다. */
	void ApplyLegacyMainWeaponBoxCollisions();

	/** AN_End_BowString — MainWeapon 이 활일 때 AttachBowString = true. */
	void ApplyLegacyEndBowStringAttach();

	FORCEINLINE constexpr bool IsMagicEquipped() const { return bMagicEquipped; }

	/** 현재 선택된 마법 EMagicType (비장착 시 Unarmed). AnimBP 의 Magic Type 핀과 맞춘다. */
	FORCEINLINE constexpr CEMagicType GetMagicEquipType() const { return MagicEquipType; }

	/** 마법 장착 중인 탈출구: 이전에 확정했던 물리 무기(허리·등에 걸린 주 무기 레이어 근사). */
	FORCEINLINE constexpr CEWeaponType GetLastCommittedPhysicalType() const { return LastCommittedPhysical; }

	FORCEINLINE constexpr CEWeaponType GetPhysicalType() const { return PhysicalType; }

private:
	void ChangePhysicalType(CEWeaponType InNewPhysical);
	void DispatchEquippedMagicDelegates(CEMagicType CurrentMagic, CEMagicType PreviousMagic);
	bool IsIdleStateMode();

	/** GetWeaponAsset·InputAction_Act 공통 — 마법·Unarmed·BP 레인 불일치 해석. */
	CEWeaponType ResolveCombatWeaponLookupType() const;

	/** BP BeginPlay — Main/Sub/Armor 클래스 배열에서 스폰 액터 채움. */
	void SpawnConfiguredWeapons();

public:
	/** DataAssets 비어 있으면 동료 WeaponComp·/Game/Weapons/CDA_* 에서 채운다. */
	void EnsureDataAssets();

	/** DataAssets → WeaponAssetMap / MagicAssetMap 재구성. */
	void RebuildAssetMaps();

	/** Owner 캐시 갱신 → DataAssets → Map 재구성(입력·BeginPlay 공통). */
	void EnsureWeaponPipelineReady();

	/** Physical=Unarmed 이면 Fist(또는 MainType) 장착 — Idle 검사 없이 SetMode 호출. */
	void EnsureCombatWeaponEquipped();

	/** 파이프라인 상태 1줄 로그(디버그). */
	void LogWeaponPipelineStatus(const TCHAR* Context);

	/** MainType 무장인데 Physical 만 Unarmed 일 때 동기화(LMB 무반응 방지). */
	void SyncBpWeaponLanes();

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

	TWeakObjectPtr<UCWeaponAsset> GetWeaponAsset();
	TObjectPtr<ACAttachment> GetAttachment();
	TObjectPtr<UCEquipment> GetEquipment();
	TObjectPtr<UCAct> GetAct();
	TObjectPtr<UCSkill> GetSkill(const int32 SkillIndex);

	/** ANS_Collision 레거시 경로 — 스폰된 MainWeapon(BP Combo/RandomPattern 등)에 OnCollisions·OffCollisions 가 있으면 호출 후 true 를 돌린다. */
	bool TryDispatchLegacyMainWeaponCollisionToggle(bool bCollisionOn);

	/** ANS_Collision·Begin/End_Bound 공통 — MainWeapon ProcessEvent 후 실패 시 ACAttachment On/OffCollisions. */
	void ApplyLegacyMainWeaponCollisionBound(bool bCollisionOn);

	/** ANS_Combo 폴백 — UCAct_Combo 없을 때 MainWeapon(Weapon_Combo) EnableCombo/DisableCombo. */
	void ApplyLegacyMainWeaponComboWindow(bool bEnableCombo);

	//
	// 레거시 BP WeaponComponent 가 스폰된 Weapon_C 액터로 직접 보던 커스텀 이벤트(ProcessEvent 규약).
	// 애니 노티파이·캐릭터 블루프린트에서 호출 가능 — 해당 UFunction 이름이 무기 블루프린트에 없으면 Verbose 로만 남긴다.
	//

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Main DoAction")
	void LegacyBp_DispatchMain_DoAction(CEAttackType InAttackType, int32 InSkillIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Main Begin DoAction")
	void LegacyBp_DispatchMain_BeginDoAction(CEAttackType InAttackType);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Main End DoAction")
	void LegacyBp_DispatchMain_EndDoAction(CEAttackType InAttackType);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Main Pressed")
	void LegacyBp_DispatchMain_Pressed();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Main Released")
	void LegacyBp_DispatchMain_Released();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Main Dash")
	void LegacyBp_DispatchMain_Dash();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Main End Ground Dash")
	void LegacyBp_DispatchMain_EndGroundDash();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Main Air Dash")
	void LegacyBp_DispatchMain_AirDash();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Main End Air Dash")
	void LegacyBp_DispatchMain_EndAirDash();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Main Skill")
	void LegacyBp_DispatchMain_Skill(const int32 InSkillIndex);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Main End Skill")
	void LegacyBp_DispatchMain_EndSkill();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Sub Hold SubWeapon")
	void LegacyBp_DispatchSub_HoldSubWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Sub Released SubWeapon")
	void LegacyBp_DispatchSub_ReleasedSubWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Sub DoSubWeapon Action")
	void LegacyBp_DispatchSub_DoSubWeaponAction(CEAttackType InAttackType, int32 InSkillIndex);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Sub Begin Do SubWeapon Action")
	void LegacyBp_DispatchSub_BeginDoSubWeaponAction(CEAttackType InAttackType);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Legacy — Sub End Do SubWeapon Action")
	void LegacyBp_DispatchSub_EndDoSubWeaponAction(CEAttackType InAttackType);

	/** BP WeaponComponent_EventGraph 의 Begin_Equip 와 동일하게 주 무기(Main_Or_Sub=true) 다음 보조(false) 순으로 디스패치한다. */
	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Begin Equip")
	void Begin_Equip();

	/** BP WeaponComponent_EventGraph 의 End_Equip 과 동일한 주→보조 순서(스폰된 Weapon 액터 쪽 함수 시그니처는 bool 또는 무인자). */
	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "End Equip")
	void End_Equip();

	/** BP WeaponComponent_EventGraph 의 Begin_Unequip 과 동일한 주→보조 순서. */
	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "Begin Unequip")
	void Begin_Unequip();

	/** BP WeaponComponent_EventGraph 의 End_Unequip 과 동일 — End_Unequip/Unequip 게이트는 DispatchLegacyUnequipOrEndUnequipGate 규약. */
	UFUNCTION(BlueprintCallable, Category = "Weapons|Legacy Spawned", DisplayName = "End Unequip")
	void End_Unequip();

	FORCEINLINE constexpr CEWeaponType GetType() const
	{
		return bMagicEquipped ? CEWeaponType::Unarmed : PhysicalType;
	}

	FORCEINLINE constexpr CEWeaponType GetPrevType() const { return PrevType; }

	/** 블루프린트 MainType — 물리 슬롯 전환 때만 동기화(마법 장착은 PhysicalType 만 Unarmed 로 두고 레이아웃 참고값으로 유지될 수 있음). */
	FORCEINLINE constexpr CEWeaponType GetMainWeaponLaneType() const { return MainType; }

	/** 블루프린트 SubType — 현재 코드 경로에서는 직전 주무기 레이어(애니 SubWeaponType 블렌딩 근사)로 갱신한다. */
	FORCEINLINE constexpr CEWeaponType GetSubWeaponLaneType() const { return SubType; }

	FORCEINLINE constexpr bool IsUnarmedMode() const
	{
		return (false == bMagicEquipped) && (PhysicalType == CEWeaponType::Unarmed);
	}

	FORCEINLINE constexpr bool IsUnarmedPhysical() const
	{
		return PhysicalType == CEWeaponType::Unarmed;
	}

	FORCEINLINE constexpr bool IsFistMode() const { return GetResolvedPhysicalStyle() == CEWeaponType::Fist; }
	FORCEINLINE constexpr bool IsSwordMode() const { return GetResolvedPhysicalStyle() == CEWeaponType::Sword; }
	FORCEINLINE constexpr bool IsHammerMode() const { return GetResolvedPhysicalStyle() == CEWeaponType::Hammer; }
	FORCEINLINE constexpr bool IsDualMode() const { return GetResolvedPhysicalStyle() == CEWeaponType::Dual; }
	FORCEINLINE constexpr bool IsBowMode() const { return GetResolvedPhysicalStyle() == CEWeaponType::Bow; }
	FORCEINLINE constexpr bool IsShieldMode() const { return GetResolvedPhysicalStyle() == CEWeaponType::Shield; }

	FORCEINLINE constexpr bool IsWarpMode() const { return bMagicEquipped && MagicEquipType == CEMagicType::Warp; }
	FORCEINLINE constexpr bool IsAroundMode() const { return bMagicEquipped && MagicEquipType == CEMagicType::Around; }
	FORCEINLINE constexpr bool IsFireballMode() const { return bMagicEquipped && MagicEquipType == CEMagicType::FireBall; }
	FORCEINLINE constexpr bool IsBombMode() const { return bMagicEquipped && MagicEquipType == CEMagicType::Bomb; }
	FORCEINLINE constexpr bool IsYonduMode() const { return bMagicEquipped && MagicEquipType == CEMagicType::Yondu; }

	/**
	 * BP WeaponComponent:IsUnarmed — MainType == Unarmed (레인 표기 전용).
	 * 장착 DA 기준 IsUnarmedMode / 주먹·마법 장착 등과 목적이 다르다 — BP 그래프와 동명으로 둔다.
	 */
	UFUNCTION(BlueprintPure, Category = "Weapons|Blueprint|Mode")
	bool IsUnarmed() const;

	/** BP WeaponComponent:IsBow — MainType == Bow(NewEnumerator 순서는 CEWeaponType 과 동일하다고 둔다). */
	UFUNCTION(BlueprintPure, Category = "Weapons|Blueprint|Mode")
	bool IsBow() const;

	UFUNCTION(BlueprintCallable, Category = "Weapons|Blueprint|Utility")
	void GetMainType(CEWeaponType& OutMainType) const;

	/** 레거시 BP 의 ChangeType — MainType/SubType 멤버와 OnWeaponTypeChanged 만 갱신(Equipment/PhysicalType 과 별개). */
	UFUNCTION(BlueprintCallable, Category = "Weapons|Blueprint|Utility", DisplayName = "ChangeType")
	void ChangeBlueprintWeaponLanes(CEWeaponType InNewMainType, CEWeaponType InNewSubType);

	/**
	 * 스폰된 BP Weapon 액터에 Unequip → ChangeType(Unarmed×2). MainWeapon/참조는 BP 와 동일하게 유지된다.
	 * 기존 SetUnarmedMode() (데이터 에셋 Equipment) 과는 목적이 다르다 — Blueprint 표시명만 SetUnarmed 로 맞춘다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapons|Blueprint|Mode", DisplayName = "SetUnarmed")
	void SetBlueprintUnarmed_WithSpawnedWeapons();

	/**
	 * BP SetMode 의 Conv_ByteToInt(InMainType) 슬롯 규약: 배열 인덱스가 enum 의 기저 uint8 과 같다고 가정한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapons|Blueprint|Utility", DisplayName = "SetMode")
	void SetBlueprintSpawnedWeaponMode(CEWeaponType InMainType, CEWeaponType InSubType);

	UFUNCTION(BlueprintCallable, Category = "Weapons|Blueprint|Mode", DisplayName = "SetSword")
	void Blueprint_SetSword_WithSpawnedWeapons();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Blueprint|Mode", DisplayName = "SetFist")
	void Blueprint_SetFist_WithSpawnedWeapons();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Blueprint|Mode", DisplayName = "SetHammer")
	void Blueprint_SetHammer_WithSpawnedWeapons();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Blueprint|Mode", DisplayName = "SetBow")
	void Blueprint_SetBow_WithSpawnedWeapons();

	UFUNCTION(BlueprintCallable, Category = "Weapons|Blueprint|Mode", DisplayName = "SetDual")
	void Blueprint_SetDual_WithSpawnedWeapons();

private:
	void RefreshOwnerCache();
	void MergeDataAssetsFromSiblingComponents();
	void CreateSyntheticDataAssetsIfStillEmpty();

	/** 물리 애니 레이어(활 조준 등)는 장착 슬롯이 마법이어도 직전 주무기 타입으로 판별한다. */
	FORCEINLINE constexpr CEWeaponType GetResolvedPhysicalStyle() const { return LastCommittedPhysical; }

	// 데이터 에셋을 객체 공유 없이 깊은 복사로 붙인다(Content DA → 런타임 전용 사본).
	UPROPERTY(EditAnywhere, Category = "DataAsset")
	TArray<TObjectPtr<UCWeaponAsset>> DataAssets;

	UPROPERTY(VisibleAnywhere, Category = "DataAsset")
	TMap<CEWeaponType, TObjectPtr<UCWeaponAsset>> WeaponAssetMap;

	UPROPERTY(VisibleAnywhere, Category = "DataAsset")
	TMap<CEMagicType, TObjectPtr<UCWeaponAsset>> MagicAssetMap;

	UPROPERTY(EditAnywhere, Category = "Settings")
	CEWeaponType PhysicalType = CEWeaponType::Unarmed;

	/** 현재 활성 무기 종류와 분리되는 마법 전용 상태. 블루프린트 EMagicType 과 같은 축이다. */
	UPROPERTY(EditAnywhere, Category = "Settings")
	CEMagicType MagicEquipType = CEMagicType::Unarmed;

	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bMagicEquipped = false;

	UPROPERTY(EditAnywhere, Category = "Settings")
	CEWeaponType LastCommittedPhysical = CEWeaponType::Unarmed;

	UPROPERTY(EditAnywhere, Category = "Settings")
	CEWeaponType PrevType = CEWeaponType::Max;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons|Blueprint", meta = (AllowPrivateAccess = "true"))
	CEWeaponType MainType = CEWeaponType::Unarmed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons|Blueprint", meta = (AllowPrivateAccess = "true"))
	CEWeaponType SubType = CEWeaponType::Unarmed;

	// 블루프린트 /Game/Weapons/Weapon.Weapon_C 레퍼런스용 클래스 배열(BP 무기 블루프린트를 그대로 담는다).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons|Blueprint", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<AActor>> MainWeaponClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons|Blueprint", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<AActor>> SubWeaponClasses;

	// 활성 무기 액터(네이티브는 ACAttachment 로 분해되어 있지만, 레거시 BP 무기 오브젝트 슬롯은 유지).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons|Blueprint", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> MainWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons|Blueprint", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AActor>> MainWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons|Blueprint", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> SubWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons|Blueprint", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AActor>> SubWeapons;

	// 블루프린트 /Game/BPs/Armors/Armor.Armor_C 대응(아머 C++ 클래스가 생기면 TSubclassOf 로 좁히면 된다).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor|Blueprint", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<AActor>> ArmorClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor|Blueprint", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> Armor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor|Blueprint", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AActor>> Armors;

public:
	UPROPERTY(BlueprintAssignable, Category = "Weapons")
	FWeaponTypeChanged OnWeaponTypeChanged;
	UPROPERTY(BlueprintAssignable)
	FMagicTypeChangedMulticast OnMagicTypeChanged;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCStateComponent> StateComp;
	TWeakObjectPtr<UCMovementComponent> MovementComp;
};
