#pragma once

#include "CoreMinimal.h"
#include "Commons/CEnums.h"
#include "Weapons/CWeaponStructures.h"
#include "Weapons/CWeapon.h"
#include "Engine/HitResult.h"
#include "Components/PrimitiveComponent.h"
#include "TimerManager.h"
#include "CWeaponCombo.generated.h"

class ACCommonCharacter;
class UAnimMontage;
class UShapeComponent;
class USoundAttenuation;
class USoundBase;
class ACharacter;
// 레거시 `/Game/Weapons/Weapon_Combo` — 콤보 인덱스·충돌 Shape·그룹/히트 처리까지 C++ 기본 구현 제공.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponCombo : public ACWeapon
{
	GENERATED_BODY()

public:
	ACWeaponCombo();

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

	// ICombatActionHost — CWeaponComponent 가 ProcessEvent 대신 우선 호출.
	virtual void Host_OnCollisions() override;
	virtual void Host_OffCollisions() override;
	virtual void Host_OnBoxCollisions() override;
	virtual void Host_EnableCombo() override;
	virtual void Host_DisableCombo() override;

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
		ACWeapon* Weapon,
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
class YJJACTIONCPPUE5_API ACWeaponGuard : public ACWeapon
{
	GENERATED_BODY()

public:
	ACWeaponGuard();

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
UCLASS(Blueprintable, meta=(DisplayName="CWeaponRandomPattern"))
class YJJACTIONCPPUE5_API ACWeaponRandomPattern : public ACWeapon
{
	GENERATED_BODY()

public:
	ACWeaponRandomPattern();

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

	virtual void Unequip_Implementation() override;

	virtual void Begin_Equip_Implementation(bool bMainOrSubWeapon) override;

	/** 레거시 RandomPattern_Sword — BeginPlay·Unequip 시 Actor Root 를 Holster 소켓에 부착. None 이면 생략. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|RandomPattern|Attach")
	FName HolsterAttachSocketName = NAME_None;

	/** 레거시 RandomPattern_Sword — Begin_Equip 시 Actor Root 를 Hand 소켓에 부착. None 이면 생략. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|RandomPattern|Attach")
	FName HandAttachSocketName = NAME_None;

	UFUNCTION(BlueprintCallable, Category = "Weapon|RandomPattern|Collision")
	void OnCollisions();

	UFUNCTION(BlueprintCallable, Category = "Weapon|RandomPattern|Collision")
	void OffCollisions();

	UFUNCTION(BlueprintCallable, Category = "Weapon|RandomPattern")
	void EnableCombo();

	UFUNCTION(BlueprintCallable, Category = "Weapon|RandomPattern")
	void DisableCombo();

	virtual void Host_OnCollisions() override;
	virtual void Host_OffCollisions() override;
	virtual void Host_EnableCombo() override;
	virtual void Host_DisableCombo() override;

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

	USkeletalMeshComponent* RandomPattern_ResolveOwnerCharacterMesh() const;
	void RandomPattern_AttachActorRootToCharacterSocket(const FName InSocketName) const;
	void RandomPattern_ApplyHolsterAttachIfConfigured() const;
	void RandomPattern_ApplyHandAttachIfConfigured() const;
};

// 레거시 `/Game/Weapons/Sword/RandomPattern_Sword` — SK_Sword·Capsule + Holster/Hand Root 부착(EventGraph 제거용).
UCLASS(Blueprintable, meta=(DisplayName="CWeaponRandomPatternSword"))
class YJJACTIONCPPUE5_API ACWeaponRandomPatternSword : public ACWeaponRandomPattern
{
	GENERATED_BODY()

public:
	ACWeaponRandomPatternSword();
};

// 레거시 `/Game/Weapons/Fist/Combo_Fist` — Shape(Fist_*Hand/Foot) 를 캐릭터 Mesh 소켓(컴포넌트 이름)에 부착. BP EventGraph BeginPlay 제거용.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponComboFist : public ACWeaponCombo
{
	GENERATED_BODY()

public:
	ACWeaponComboFist();

	virtual void BeginPlay() override;

private:
	void ComboFist_AttachCollisionShapesToCharacterMesh();
};