#pragma once

#include "CoreMinimal.h"
#include "Commons/CEnums.h"
#include "Weapons/CWeaponStructures.h"
#include "Engine/HitResult.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Components/SceneComponent.h"
#include "Components/CMovementComponent.h"
#include "CSkillWeapon.generated.h"

class ACCommonCharacter;
class ACPlayableCharacter;
class UCStateComponent;
class UDataTable;
class USoundAttenuation;
class USoundBase;
class UPrimitiveComponent;
class UShapeComponent;
class UAnimMontage;
class ACharacter;

// BP Magic / Weapon 이 공유하는 스킬 주체 캐릭터 슬롯.
UCLASS(Abstract)
class YJJACTIONCPPUE5_API ACSkillContextProvider : public AActor
{
	GENERATED_BODY()

public:
	// Magic/Weapon BP BeginPlay 에서 Owner 를 Cast 해 채우므로 블루프린트에서 쓰기 허용한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TObjectPtr<ACCommonCharacter> Character;
};

// BP `/Game/Magics/Magic` 가 달도록 하는 마커 부모(첫 번째 Cast 분기).
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACMagicSkillContext : public ACSkillContextProvider
{
	GENERATED_BODY()

public:
	ACMagicSkillContext();

protected:
	virtual void BeginPlay() override;

	// 레거시 Magic EventGraph ReceiveTick — 무기 ACSkillWeapon 과 동일하게 탑승 시 Moving 을 바꾼다.
	virtual void Tick(float DeltaSeconds) override;

public:
	/** 레거시 Set Component 그래프 — 캐릭터 이동(프로젝트 UCMovementComponent , BP 이름 Moving 과 동등). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic|Components")
	TObjectPtr<UCMovementComponent> Moving;

	// BP Magic_C::Equip — C++ 에서 장착 훅 필요 시 여기 또는 BP 에서 재정의한다.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Magic")
	void Equip();
	virtual void Equip_Implementation();

	// BP Magic_C::Unequip — SetUnarmed / SetMode 가 슬롯 전환 전에 호출한다.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Magic")
	void Unequip();
	virtual void Unequip_Implementation();

	// ---------- BP Magic_C — MagicComponent EventGraph 가 캐릭터 애니·입력에서 포워드 ----------

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Magic")
	void Begin_Equip();
	virtual void Begin_Equip_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Magic")
	void End_Equip();
	virtual void End_Equip_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Magic")
	void Begin_Unequip();
	virtual void Begin_Unequip_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Magic")
	void End_Unequip();
	virtual void End_Unequip_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Magic")
	void DoAction(CEAttackType InAttackType, int32 InSkillIndex);
	virtual void DoAction_Implementation(CEAttackType InAttackType, int32 InSkillIndex);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Magic")
	void Begin_DoAction(CEAttackType InAttackType);
	virtual void Begin_DoAction_Implementation(CEAttackType InAttackType);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Magic")
	void End_DoAction(CEAttackType InAttackType);
	virtual void End_DoAction_Implementation(CEAttackType InAttackType);

	// 레거시 Magic_C::PlayAction — 마나 차감(플레이어만)·몽타주·Moving·이펙트·모션 트레일까지 Weapon PlayAction 과 동등 분기.
	UFUNCTION(BlueprintCallable, Category = "Magic|Action")
	void PlayAction(const TArray<FDoActionData>& InDatas, int32 InIndex);

	UFUNCTION(BlueprintCallable, Category = "Magic|Commons")
	void RestorePrevState();

	/** Warp::SetCandidate 등 — UCStateComponent::IsRealRiding 만 있는데 레거시 BP 가 State UObject·Magic Self 에 붙여 두어 깨지는 분기 대체(Self = 매직 컨텍스트 액터). */
	UFUNCTION(BlueprintPure, Category = "Magic|State", meta = (DisplayName = "Is Character Real Riding"))
	bool Magic_IsCharacterRealRiding() const;

	/** State(UObject) 슬롯이 비었거나 클래스 리팩토 후 깨졌을 때 Character 에서 UCStateComponent 를 찾아 갱신(레거시 State Get 핀 복구·순회 호출 빈도는 낮다고 가정). */
	UFUNCTION(
		BlueprintCallable,
		Category = "Magic|State",
		meta = (DisplayName = "Refresh State Reference From Character Component"))
	void Magic_RefreshStateReferenceFromCharacter();

	/** Equip/UnEquip(Warp)·타깃 미리보기 — 탑승 시 CurInteractingActor 쪽 이동 컴포넌트 우선, 없으면 캐릭터 검색(레거시 ComponentClass→FixCamera 핀 대체). */
	UFUNCTION(BlueprintCallable, Category = "Magic|Camera", meta = (DisplayName = "Fix Camera (Owner Movement)"))
	void Magic_FixCameraForOwnerMovement();

	/** Equip 그래프 짝 — UnFixCamera 를 동일 해석 경로로 호출. */
	UFUNCTION(BlueprintCallable, Category = "Magic|Camera", meta = (DisplayName = "Un Fix Camera (Owner Movement)"))
	void Magic_UnFixCameraForOwnerMovement();

	/** DoAction 등 — State 컴포넌트 SetAction 을 Magic Self 경로로 호출(레거시 State 핀·Magic_Warp 대상 노드 대체). */
	UFUNCTION(BlueprintCallable, Category = "Magic|State", meta = (DisplayName = "Set Action (Magic Owner)"))
	void Magic_SetOwnerStateAction();

	/** DoAction 조건 분기 — UCStateComponent::IsIdle 로 위임(State UObject 핀 제거용). */
	UFUNCTION(BlueprintPure, Category = "Magic|State", meta = (DisplayName = "Is Idle (Magic Owner)"))
	bool Magic_IsOwnerIdle() const;

	/** DoAction 조건 분기 — UCStateComponent::IsRiding 로 위임. */
	UFUNCTION(BlueprintPure, Category = "Magic|State", meta = (DisplayName = "Is Riding (Magic Owner)"))
	bool Magic_IsOwnerRiding() const;

	/** DoAction 조건 분기 — UCStateComponent::IsAction 로 위임. */
	UFUNCTION(BlueprintPure, Category = "Magic|State", meta = (DisplayName = "Is Action (Magic Owner)"))
	bool Magic_IsOwnerInAction() const;

	UFUNCTION(
		BlueprintCallable,
		Category = "Magic|Commons",
		meta = (ReturnDisplayName = "Enough Mana", DisplayName = "Consume Mana"))
	bool ConsumeMana(double InMana);

	UFUNCTION(BlueprintCallable, Category = "Magic|Commons", meta = (DisplayName = "Destroy"))
	void Destroy_FromMagicBlueprint();

	/** 레거시 Magic_C::SetControllerInEquip — 무기는 Character 직접 참조를 쓰지만, BP 는 Owner 캐스트로 CurController 를 복사했다. */
	UFUNCTION(BlueprintCallable, Category = "Magic|Equip")
	void SetControllerInEquip();

	// ---------- 레거시 `/Game/Magics/Magic` BPVar — Weapon 과 동형 데이터·플래그(매직 전용 카테고리) ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|DataTables")
	TObjectPtr<UDataTable> EquipDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|DataTables")
	FEquipData EquipData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|DataTables")
	FEquipData UnequipData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|DataTables")
	TObjectPtr<UDataTable> DoActionDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|DataTables")
	TArray<FDoActionData> DoActionDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|DataTables")
	TObjectPtr<UDataTable> HitDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|DataTables")
	TArray<FHitData> HitCommonDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Equip")
	bool IsEquipping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Equip")
	bool InEquip = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Equip")
	bool IsUnequipping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Action")
	bool InAction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Settings")
	TObjectPtr<AController> Controller;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Settings")
	CEAttackType PrevAttackType = CEAttackType::Common;

	// 레거시 BP 타입 무지정 UObject — 보통 UCStateComponent (BeginPlay 에서 채운다).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Default")
	TObjectPtr<UObject> State;

private:
	// 레거시 EventGraph 실행 순서: Cache 비우기 → Character/Moving/State → 표 적재(Common 분기만).
	void Magic_ClearRuntimeTableCaches();
	void Magic_SetCharacterAndComponentsFromBlueprint();
	void Magic_LoadEquipDatasFromTable();
	void Magic_LoadDoActionDatasFromTable_CommonOnly();
	void Magic_LoadHitDatasFromTable_CommonOnly();

	/** ACWeaponSkillContext::Weapon_ApplyEquipRowFacingAndMovement 와 동일 분기를 Magic 전용 로그 문자열로 재사용. */
	void Magic_ApplyEquipRowFacingAndMovement(const FEquipData& Row);

	/** 레거시 Weapon_PlayAction 후반 시퀀스와 동일 — 이펙트/사운드 위치는 캐릭터 발 기준으로 둔다(Magic 그래프 K2_GetActorLocation). */
	void Magic_PlayActionRunExecutionBranches(const FDoActionData& Row, ACPlayableCharacter* OptionalPlayableActor);

protected:
	/** State UObject 또는 Character 조회로 UCStateComponent 해석(SetEquip 과 동형 — Magic_Around 등 확장 클래스에서 상태 분기에 사용). */
	UCStateComponent* Magic_ResolveStateComponent() const;

	/** Tick 이 갱신한 Moving 캐시 우선, 없으면 탑승 상대·캐릭터에서 UCMovementComponent 탐색. */
	UCMovementComponent* Magic_ResolveMovementForOwnerOrMount() const;
};

// 레거시 `/Game/Magics/Around/Magic_Around` — 어라운드 볼 클래스 배열 무작위 스폰(Begin_DoAction) + 조건 충족 시 SetAction 후 PlayAction.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACMagicAroundSkillContext : public ACMagicSkillContext
{
	GENERATED_BODY()

public:
	ACMagicAroundSkillContext();

	/** 레거시 Magic_Around_C::SkillClasses — Skill_Around 파생 블루프린트 클래스 목록(BP 디폴트와 동일하게 에디터에서 채움). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Around")
	TArray<TSubclassOf<AActor>> SkillClasses;

	virtual void DoAction_Implementation(CEAttackType InAttackType, int32 InSkillIndex) override;
	virtual void Begin_DoAction_Implementation(CEAttackType InAttackType) override;

private:
	void Around_SpawnRandomSkillActorFromSkillClasses();

};

// 레거시 `/Game/Weapons/Weapon` 액터 C++ 매핑 — BPVar·루트 컴포넌트 + BeginPlay 에서 채우는 레퍼런스(Moving 등).
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponSkillContext : public ACSkillContextProvider
{
	GENERATED_BODY()

public:
	ACWeaponSkillContext();

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

	static void Weapon_AppendDoActionRowByAttackType(ACWeaponSkillContext* InWeapon, const FDoActionData& InRow);

	/** DoMovingActionDataTable 분기만 — 레거시 Dash / AirDash 만 DoMovingActionDatas 로 적재한다. */
	static void Weapon_AppendMovingDoActionRowIfDashTypes(ACWeaponSkillContext* InWeapon, const FDoActionData& InRow);

	static void Weapon_AppendHitRowByAttackType(ACWeaponSkillContext* InWeapon, const FHitData& InRow);

	/** 레거시 Equip 표 분기 — UseControlRotation / CanMove 에 따라 이동 컴포넌트만 갱신한다. */
	void Weapon_ApplyEquipRowFacingAndMovement(const FEquipData& Row);

	/** Weapon.State 또는 캐릭터에서 UCStateComponent 해석 — 대시·낙하 분기에 공통 사용. */
	UCStateComponent* Weapon_ResolveStateComponent() const;

	/** State 또는 CharacterMovement 로 공중(낙하·비행) 여부 — Dash 의 AirDash 분기. */
	bool Weapon_ShouldAirDashFromMovementState() const;

	/** 레거시 Weapon::DoAction 본체 — 콤보 무기(ACWeaponComboSkillContext) 에서 타입 디스패치를 덮어쓴다. */
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

// 레거시 `/Game/Weapons/Weapon_Combo` — 콤보 인덱스·충돌 Shape·그룹/히트 처리까지 C++ 기본 구현 제공.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponComboSkillContext : public ACWeaponSkillContext
{
	GENERATED_BODY()

public:
	ACWeaponComboSkillContext();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Action")
	CEAttackType CurAttackType = CEAttackType::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Actions", meta = (DisplayName = "Enable"))
	bool ComboEnable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Actions", meta = (DisplayName = "Exist"))
	bool ComboExist = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Actions", meta = (DisplayName = "Common Index"))
	int32 ComboHitIndexCommon = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Actions", meta = (DisplayName = "Air Index"))
	int32 ComboHitIndexAir = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Actions", meta = (DisplayName = "Flying Index"))
	int32 ComboHitIndexFlying = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Actions", meta = (DisplayName = "Down Index"))
	int32 ComboHitIndexDown = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Actions", meta = (DisplayName = "Swaying Index"))
	int32 ComboHitIndexSwaying = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Actions", meta = (DisplayName = "Fall Down Index"))
	int32 ComboHitIndexFallDown = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Actions", meta = (DisplayName = "Cur Skill Index"))
	int32 ComboHitIndexSkill = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Targeting")
	FRotator ComboRotationTarget = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Targeting")
	bool ComboTargetingFlag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Hit", meta = (DisplayName = "Mouse Position"))
	FVector2D ComboMousePosition = FVector2D::ZeroVector;

	/** 레거시 기본 피격 몽타주 블레이드 변수 — 행별 `FHitData::Montage` 가 우선한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combo|Hit")
	TObjectPtr<UAnimMontage> ComboDefaultHitReactionMontage = nullptr;

	/** 레거시 `BF_Helpers::GetHitPoint` 구체 반경 근사. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Combo|Hit", meta = (ClampMin = "1.0"))
	float ComboMeleeHitSphereRadius = 48.f;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Weapon|Combo|Hit",
		meta = (DisplayName = "Collisions"))
	TArray<TObjectPtr<UShapeComponent>> ComboCollisionShapes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Hit")
	TArray<TObjectPtr<ACCommonCharacter>> ComboHitted;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaSeconds) override;

	/** 레거시 ANS·WeaponComponent 호출 가능 — 활성 무기 블루프린트에도 동일 이름 권장. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Combo")
	void OnCollisions();

	/** 레거시 AN_BoxCollision — ComboCollisionShapes 중 Box 만 QueryAndPhysics. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Combo")
	void OnBoxCollisions();

	/** 레거시 ANS 종료 분기 및 BeginPlay 초기 상태. 피격 누적(`ComboHitted`)을 비운다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Combo")
	void OffCollisions();

	/** 레거시 Custom Event EnableCombo — 콤보 입력/체인 허용. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Combo")
	void EnableCombo();

	/** 레거시 Custom Event DisableCombo. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Combo")
	void DisableCombo();

	/** 레거시 `Weapon_Combo:Begin_DoFlyingAttack` 애님 노티파이 등 — `Flying` 행 진행 및 `AddGravity`. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Combo", meta = (DisplayName = "Begin Flying Segment (Legacy Combo)"))
	void Combo_BeginFlyingSegmentFromNotify();

	/**
	 * 레거시 BP `Weapon_Combo:DoFallDownAttack` 의 `FallDownGravity` 문자열 타머 근거.
	 * 블루프린트/문자열 `K2_SetTimer` 와 이름을 맞추기 위해 같은 식별자로 둔다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Combo")
	void FallDownGravity();

	/** 레거시 `Weapon_Combo:OnBeginOverlap` — 블루프린트에서 추가 이펙트를 얹을 수 있게 NativeEvent 로 둠. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Combo")
	void Combo_OnBeginOverlap(ACCommonCharacter* InOtherCharacter, FVector InHitPoint);

	virtual void End_DoAction(CEAttackType InAttackType) override;

protected:
	virtual void Weapon_DoActionImpl(CEAttackType InAttackType, int32 InSkillIndex) override;

	virtual void Begin_DoAction_Implementation(CEAttackType InAttackType) override;
	/** 레거시 BP 에서 타입별로 연결돼 있던 Begin_* 종료 분기 후크 — 서브클래스/후속 포팅 확장점. */
	virtual void ComboFork_Begin_Common();
	virtual void ComboFork_Begin_Air();
	virtual void ComboFork_Begin_Flying();

	virtual void ComboFork_End_Common();
	virtual void ComboFork_End_Air();
	virtual void ComboFork_End_Flying();
	virtual void ComboFork_End_Down();
	virtual void ComboFork_End_DashAttack();
	virtual void ComboFork_End_FallDown();
	virtual void ComboFork_End_Skill();
	virtual void ComboFork_End_Riding();

	UFUNCTION()
	void ComboOnShapeBeginOverlap_Impl(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepFromMaybe);

	UFUNCTION()
	void ComboOnShapeHit_Impl(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

private:
	void Combo_PlayDatasOrWarn(
		const TArray<FDoActionData>& InDatas,
		const int32 InPreferredIndex,
		const TCHAR* BranchLabelForLog);
	void Combo_DispatchDoAction(CEAttackType InAttackType, int32 InSkillIndex);

	bool Combo_AttemptApplyHitAgainst(
		UPrimitiveComponent* WeaponPrimitive,
		UPrimitiveComponent* OtherPrim,
		ACCommonCharacter* HitCharacter,
		const FVector& InHitWorldLocation);

	static bool Combo_ResolveHitDatasForMelee(
		ACWeaponSkillContext* Weapon,
		const CEAttackType AttackTypeBranch,
		int32 IndexCommon,
		int32 IndexAir,
		int32 IndexFlying,
		int32 IndexDown,
		int32 IndexSwaying,
		int32 IndexFallDown,
		int32 IndexSkill,
		TArray<FHitData>*& OutHitDatasPtr,
		int32& OutClampedHitIndex);

	/** 레거시 Tick: 장착 중이면 타게팅 보간 — 소유자 `UCTargetingComponent` 에 위임한다(중복 틱 회전 방지 위해 컴포넌트 활성 시 생략). */
	void Combo_Tick_TargetingDelegation();

	/** `OffCollisions`(고정 카메라)에서 켠 레거시 `Weapon_Combo:Tick_Targeting` — `RotationTarget` 보간 후 마우스/각도 종료 검사 → `Targeting` 해제. */
	void Combo_Tick_FixedCameraLookTarget_MouseEnd();

	bool Combo_ApplyLegacyDoAirComboAfterBeginGuards(const TCHAR* BranchLabelForLog);
	bool Combo_ApplyLegacyDoFlyingAttackAfterBeginGuards(const TCHAR* BranchLabelForLog);

	bool Combo_ApplyLegacyDoDownAttackAfterBeginGuards(const TCHAR* BranchLabelForLog);
	bool Combo_ApplyLegacyDoDashAttackAfterBeginGuards(const TCHAR* BranchLabelForLog);
	bool Combo_ApplyLegacyDoFallDownAttackAfterBeginGuards(const TCHAR* BranchLabelForLog);

	bool Combo_ApplyLegacySkillAfterBeginGuards(const int32 InSkillIndex, const TCHAR* BranchLabelForLog);

	bool Combo_ApplyLegacyRidingAttackAfterBeginGuards(const TCHAR* BranchLabelForLog);

	/** `DoFallDownAttack` 구간에서 레거시 0.3s 지연 후 `FallDownGravity` 호출용. 타머 누수 방지 위해 `EndPlay` 에서 무효화한다. */
	FTimerHandle ComboFallDownGravityTimerHandle;
};

// 레거시 `/Game/Weapons/Weapon_Guard` BPVar · EventGraph/함수그래프 C++ 매핑.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponGuardSkillContext : public ACWeaponSkillContext
{
	GENERATED_BODY()

public:
	ACWeaponGuardSkillContext();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	/** 레거시 BP `Hold_SubWeapon` — Parriable·Parrying 분기 후 Idle/Hitted 에서만 Guarding 및 가드 연출 재생. */
	virtual void Hold_SubWeapon_Implementation() override;

	/** 레거시 BP `Released_SubWeapon` — Guarding 해제·가드 몽타주 중단·Moving.Move. */
	virtual void Released_SubWeapon_Implementation() override;

	/** 레거시 `Weapon_Guard:DoSubWeaponAction` 빈 그래프. */
	virtual void DoSubWeaponAction_Implementation() override;

	/** 레거시 `Begin_DoSubWeaponAction` — 전역 액터 순회 후 Pawn 또는 Movable 메시면 `DilationActors` 에 넣고 HitStop 만큼 시간 팽창. */
	virtual void Begin_DoSubWeaponAction_Implementation() override;

	/** 레거시 `End_DoSubWeaponAction` — Parrying 해제·누적 액터 시간 복원·배열 비우기. */
	virtual void End_DoSubWeaponAction_Implementation() override;

	/** 레거시 `Weapon_Guard:OnGuardCollision(GT_Function)`. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Guard|Collision", meta = (DisplayName = "On Guard Collision"))
	void OnGuardCollision();

	/** 레거시 BP 함수명 철자 `OffGuardCollsion`(Blueprint 호출 이름 유지). */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Guard|Collision", meta = (DisplayName = "Off Guard Collision"))
	void OffGuardCollsion();

	/** 레거시 `Weapon_Guard:OnSubWeaponCollision`. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Guard|Collision", meta = (DisplayName = "On Sub Weapon Collision"))
	void OnSubWeaponCollision();

	/** 레거시 `Weapon_Guard:OffSubWeaponCollision`. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Guard|Collision", meta = (DisplayName = "Off Sub Weapon Collision"))
	void OffSubWeaponCollision();

public:
	/** 레거시 `Weapon_Guard:OnBeginOverlap` — 가드 처리·패링 애니·패링 카운터 히트. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|Guard")
	void OnBeginOverlap(ACCommonCharacter* InCharacter, const FVector& InHitPoint);

	virtual void OnBeginOverlap_Implementation(ACCommonCharacter* InCharacter, const FVector& InHitPoint);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard")
	bool Guarding = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard")
	bool Parriable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard")
	bool Parrying = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard")
	bool Parried = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard")
	TArray<TObjectPtr<AActor>> DilationActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard|Collision", meta = (DisplayName = "Guard Collision Index"))
	int32 GuardCollisionIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard|Collision", meta = (DisplayName = "Action Collision Index"))
	int32 ActionCollisionIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard|Animation", meta = (DisplayName = "Guard Anim Index"))
	int32 GuardAnimIndex = 0;

	/** 레거시 `Released_SubWeapon` 의 `StopAnimMontage`(예: `Guard_Montage_2`). 미할당 시 `Released_SubWeapon` 에서 `ensure` 로 중단한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Guard|Animation", meta = (DisplayName = "Guard Hold Montage (Stop On Release)"))
	TObjectPtr<UAnimMontage> GuardHoldStopMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard|Animation", meta = (DisplayName = "Parrying Anim Index"))
	int32 ParryingAnimIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard")
	TObjectPtr<ACharacter> Target = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard", meta = (DisplayName = "Parriable Delay"))
	double ParriableDelay = 0.1;

	/** 패링 타격 회접(`Damaged`/SendDamage`) 전 지연 — 레거시 `ParriedDelay`. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard", meta = (DisplayName = "Parried Delay"))
	double ParriedDelay = 0.2;

	/** `TryGetHitPointBetweenPrimitives` 구체 반경 — 무기 컴보 무기 배치에 맞게 조정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard|Collision")
	float GuardHitSphereRadius = 48.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard|Collision")
	TArray<TObjectPtr<UShapeComponent>> Collisions;

	/** 레거시 `Weapon_Guard.Hitted`. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard|Hit")
	TArray<TObjectPtr<ACCommonCharacter>> Hitted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Guard|Hit", meta = (DisplayName = "Clear Hitted"))
	bool ClearHitted = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Guard|Fx", meta = (DisplayName = "Guard Block Sound (optional, content default 설정)"))
	TObjectPtr<USoundBase> GuardBlockSoundAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Guard|Fx")
	TObjectPtr<USoundAttenuation> GuardBlockAttenuation = nullptr;

private:
	void Guard_BindOverlapsRecursive();
	void Guard_UnbindOverlapsRecursive();

	UFUNCTION()
	void Guard_OnShapeBeginOverlap_Dynamic(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void Guard_Timer_ClearParriableElapsed();
	void Guard_Timer_ParriedReflectElapsed();

	/** 레거시 `ClearHitted` 브랜치 — 0.8s 후 플래그·배열 정리. */
	void Guard_Timer_ClearHittedListElapsed();

	FTimerHandle GuardParriableResetTimerHandle;
	FTimerHandle GuardParriedReflectTimerHandle;
	FTimerHandle GuardClearHittedTimerHandle;
};

class USkeletalMeshComponent;

// 레거시 `/Game/Weapons/Weapon_RandomPattern` — 랜덤 DoAction 몽타주·Shape 오버랩 피격(`HitCommonDatas[RandomIndex]`).
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponRandomPatternSkillContext : public ACWeaponSkillContext
{
	GENERATED_BODY()

public:
	ACWeaponRandomPatternSkillContext();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|RandomPattern")
	bool Enable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|RandomPattern")
	int32 RandomIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|RandomPattern")
	FRandomStream RandomSeed;

	/** `PlayRandomActionMontage` 에서 `RandomIntegerInRange` 상한(레거시 BP 기본 3). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|RandomPattern", meta = (ClampMin = "0"))
	int32 RandomPatternMontageRandomMax = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|RandomPattern|Collision")
	TArray<TObjectPtr<UShapeComponent>> Collisions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|RandomPattern|Hit")
	TArray<TObjectPtr<ACCommonCharacter>> Hitted;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|RandomPattern|Hit", meta = (ClampMin = "1.0"))
	float RandomPatternMeleeHitSphereRadius = 48.f;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon|RandomPattern|Collision")
	void OnCollisions();

	UFUNCTION(BlueprintCallable, Category = "Weapon|RandomPattern|Collision")
	void OffCollisions();

	UFUNCTION(BlueprintCallable, Category = "Weapon|RandomPattern")
	void EnableCombo();

	UFUNCTION(BlueprintCallable, Category = "Weapon|RandomPattern")
	void DisableCombo();

	UFUNCTION(BlueprintCallable, Category = "Weapon|RandomPattern|Actions")
	void PlayRandomActionMontage();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon|RandomPattern|Hit")
	void OnBeginOverlap(ACCommonCharacter* InOtherCharacter, const FVector& InHitPoint);

	virtual void End_DoAction(CEAttackType InAttackType) override;

protected:
	virtual void Weapon_DoActionImpl(CEAttackType InAttackType, int32 InSkillIndex) override;

	virtual void End_Equip_Implementation(bool bMainOrSubWeapon) override;

	UFUNCTION()
	void RandomPatternOnShapeBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

private:
	bool RandomPattern_TryRegisterHitTarget(ACCommonCharacter* InCharacter);
};

// 레거시 `/Game/Weapons/Sword/Combo_Sword` — 콤보 무기 + 검 메시 장착 소켓(`Hand_Sword`/`Holster_Sword`) 표시.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponComboSwordSkillContext : public ACWeaponComboSkillContext
{
	GENERATED_BODY()

public:
	ACWeaponComboSwordSkillContext();

	virtual void BeginPlay() override;

	virtual void Unequip_Implementation() override;

	virtual void Begin_Equip_Implementation(bool bMainOrSubWeapon) override;

	// BP 자식(Combo_Sword)의 SkeletalMesh 컴포넌트와 UPROPERTY 동명이면 reparent 시 ICE — BeginPlay 에서만 해석한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Sword")
	FName HandAttachSocketName = TEXT("Hand_Sword");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo|Sword")
	FName HolsterAttachSocketName = TEXT("Holster_Sword");

private:
	TObjectPtr<USkeletalMeshComponent> ComboSwordMesh;

	void ComboSword_ResolveSkeletalMesh();
	void ComboSword_SetMeshVisible(bool bVisible);
	void ComboSword_AttachToCharacterSocket(FName SocketName);
};

// 레거시 `/Game/Weapons/Fist/Combo_Fist` — Shape(Fist_*Hand/Foot) 를 캐릭터 Mesh 소켓(컴포넌트 이름)에 부착. BP EventGraph BeginPlay 제거용.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponComboFistSkillContext : public ACWeaponComboSkillContext
{
	GENERATED_BODY()

public:
	ACWeaponComboFistSkillContext();

	virtual void BeginPlay() override;

private:
	void ComboFist_AttachCollisionShapesToCharacterMesh();
};

// BP Skill_Weapon / Skill_Magic — ChildActor 또는 스폰 시 Owner 가 Magic(ACMagicSkillContext) 또는 Weapon(ACWeaponSkillContext) 이면
// BeginPlay 에서 아래 참조가 채워진다. 블루프린트에 동일 이름(Character/Weapon/Magic) 변수를 만들면 숨김·복제 충돌이 나므로 두지 않는다.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACSkillWeapon : public AActor
{
	GENERATED_BODY()

public:
	ACSkillWeapon();

protected:
	/** 레거시 BP Skill_Weapon / Skill_Around 의 DefaultSceneRoot 와 동일한 이름으로 자식 무기 계열이 붙는다(Sphere 등). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SkillWeapon|Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	virtual void BeginPlay() override;

public:
	// BeginPlay 채우기 전 수동 에디터 대입 허용(레거시). 스킬 BP는 중복 변수 대신 여기 연결 또는 그래프 제거 후 상속 Getter만 사용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TObjectPtr<ACMagicSkillContext> Magic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TObjectPtr<ACWeaponSkillContext> Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TObjectPtr<ACCommonCharacter> Character;
};

// BP `/Game/Magics/Skill_Magic` — 부모 `ACSkillMagic` 에서 상속 프로퍼티 Character/Magic/Weapon 만 쓸 것(블프 동명 변수 금지). BeginPlay 순서 상 C++ 채운 뒤 BP Event 가능.
// 무기 블프 `/Game/Weapons/Weapon` 은 반드시 `ACWeaponSkillContext` 계통이어야 `Character` 속성 노드가 유효하다(`ACSkillContextProvider::Character`).
UCLASS(
	Blueprintable,
	meta=(
		DisplayName="Skill Magic Actor",
		BlueprintDescription="AActor 가 아니라 ACSkillMagic(ACSkillWeapon) 을 부모로 지정해야 BeginPlay 에서 Owner(Magic/Weapon) 기준 Character 등이 채워진다."))
class YJJACTIONCPPUE5_API ACSkillMagic : public ACSkillWeapon
{
	GENERATED_BODY()

public:
	ACSkillMagic() = default;
};
