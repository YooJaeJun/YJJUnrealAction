#pragma once

#include "CoreMinimal.h"
#include "Commons/CEnums.h"
#include "Weapons/CWeaponStructures.h"
#include "Weapons/COwnerActor.h"
#include "Engine/HitResult.h"
#include "GameFramework/Controller.h"
#include "Components/CMovementComponent.h"
#include "CMagic.generated.h"

class ACPlayableCharacter;
class UCStateComponent;
class UDataTable;

// BP `/Game/Magics/Magic` 가 달도록 하는 마커 부모(첫 번째 Cast 분기).
UCLASS(Blueprintable, meta=(DisplayName="CMagic"))
class YJJACTIONCPPUE5_API ACMagic : public ACOwnerActor
{
	GENERATED_BODY()

public:
	ACMagic();

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

	/** ACWeapon::Weapon_ApplyEquipRowFacingAndMovement 와 동일 분기를 Magic 전용 로그 문자열로 재사용. */
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
class YJJACTIONCPPUE5_API ACMagicAround : public ACMagic
{
	GENERATED_BODY()

public:
	ACMagicAround();

	/** 레거시 Magic_Around_C::SkillClasses — Skill_Around 파생 블루프린트 클래스 목록(BP 디폴트와 동일하게 에디터에서 채움). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic|Around")
	TArray<TSubclassOf<AActor>> SkillClasses;

	virtual void DoAction_Implementation(CEAttackType InAttackType, int32 InSkillIndex) override;
	virtual void Begin_DoAction_Implementation(CEAttackType InAttackType) override;

private:
	void Around_SpawnRandomSkillActorFromSkillClasses();

};
