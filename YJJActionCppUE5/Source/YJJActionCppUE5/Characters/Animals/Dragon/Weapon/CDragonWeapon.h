#pragma once

#include "CoreMinimal.h"
#include "Characters/Animals/CAnimalWeapon.h"
#include "Particles/ParticleSystem.h"
#include "Animation/AnimMontage.h"
#include "Commons/CEnums.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Sound/SoundBase.h"
#include "CDragonWeapon.generated.h"

class ACDragon;
class ACCommonCharacter;
class USkeletalMeshComponent;

/**
 * 레거시 `/Game/Character/Animals/Dragon/Weapon/DragonWeapon` — 블프가 AnimalWeapon(ACAnimalWeapon) 확장 했던
 * 디자이너 튜너 변수를 네이티브에 담았다.
 * Tick_Rotate / LandAttack / Skill 분기 레거시 그래프는 C++ 에 옮김 — 개별 스킬 채널 Skill_1~6 은 블프 하위에서 오버라이드 가능.
 */
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACDragonWeapon : public ACAnimalWeapon
{
	GENERATED_BODY()

public:
	ACDragonWeapon();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaSeconds) override;

	/** 레거시 이벤트명 Skill — ACDragon::InvokeDragonWeaponSkill 과 ProcessEvent 바이트 인자 동형. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dragon|Weapon")
	void Skill(uint8 InSkillType);
	virtual void Skill_Implementation(uint8 InSkillType);

	/**
	 * 레거시 DragonWeapon 블프 몽타주 노티 경로 약속(`AN_Begin_DragonSkill` 등).
	 * 내부적으로 동일 검증 후 `Skill` 디스패치를 호출한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dragon|Weapon")
	void Begin_Skill(uint8 InSkillType);

	/**
	 * 레거시 DragonWeapon 블프 `End_Skill`(몽타주 `AN_End_DragonSkill`).
	 * 타임즈 `Skill_SchedulePhaseEnd` 와 같은 정리 플래트로 맞춰 이중 실행을 피하려 페이즈 타이머를 끊은 뒤 `Dragon_OnSkillPhaseEndTimerExpired` 경로와 동등한 정리를 수행한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dragon|Weapon")
	void End_Skill(uint8 InSkillType);

	/** 스킬 컨텍스트·외부 헬퍼용 — 멤버 `Dragon` 우선 후 `CharacterOwner` 를 ACDragon 으로 캐스트한다. */
	ACDragon* Dragon_ResolveOwningDragon() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dragon|Weapon")
	void LandAttack();
	virtual void LandAttack_Implementation();

	/** 레거시 그래프 `Skill_1` ~ `Skill_6` 과 동형 — NativeEvent 라 블루프린트 재부모 시 그래프 이식 가능. */
	UFUNCTION(BlueprintNativeEvent, Category = "Dragon|Weapon|Skills")
	void Skill_1();
	virtual void Skill_1_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Dragon|Weapon|Skills")
	void Skill_2();
	virtual void Skill_2_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Dragon|Weapon|Skills", meta = (DisplayName = "Skill 3 (Meteor)"))
	void Skill_3();
	virtual void Skill_3_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Dragon|Weapon|Skills")
	void Skill_4();
	virtual void Skill_4_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Dragon|Weapon|Skills")
	void Skill_5();
	virtual void Skill_5_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Dragon|Weapon|Skills")
	void Skill_6();
	virtual void Skill_6_Implementation();

	// 레거시 DragonWeapon 블프 카테고리에 맞춘 편성.

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "DragonWeapon|Settings",
		meta = (DisplayName = "Player", MultiLine = "true"))
	TObjectPtr<ACCommonCharacter> Player;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|LandAttack")
	TObjectPtr<UParticleSystem> LandAttackEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|LandAttack")
	TObjectPtr<USoundBase> LandAttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|LandAttack")
	TArray<TObjectPtr<ACCommonCharacter>> LandHitted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|LandAttack")
	double LandAttackDamage = 3.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|LandAttack")
	TObjectPtr<UAnimMontage> LandAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Land Attack")
	double LandAttackRadius = 400.0;

	/** 레거시 Tick_Rotate 의 RInterp 속도(블프 디폴트 1.5). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Tick Rotate")
	float RotateTowardPlayerInterpSpeed = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill")
	double MeshRadius = 200.0;

	/** 레거시 `ETeleportProcessType`(Content UDE)과 순서·값 고정 동형 (`CETeleportProcessType`). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill")
	CETeleportProcessType TeleportProcessType = CETeleportProcessType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill", meta = (DisplayName = "Skill Wait Time"))
	TArray<double> SkillWaitTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill")
	int32 CurSkillIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 1")
	int32 RoarIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 1")
	double RoarRadius = 5000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 2")
	int32 FireIndex = 2;

	/** 레거시 `Skill_Dragon_Fire` 블프 액터 — 비어 있으면 인스턴스 디폴트에서 로드하지 않으며 스폰하지 않음. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 2")
	TSubclassOf<AActor> FireSkillActorClass;

	/** 레거시 `Skill_Dragon_FireAir` 블프 액터 — `FireFlying` 그래프와 동형. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 4")
	TSubclassOf<AActor> FireFlySkillActorClass;

	/** 레거시 `Skill_Dragon_Meteor` 블프 액터 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 3")
	TSubclassOf<AActor> MeteorSkillActorClass;

	/** 레거시 `Fire_Ground` 소켓. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|SkillSockets")
	FName DragonFireAttachSocketName = FName(TEXT("Fire_Ground"));

	/** 레거시 `FireFlying` Neck·FireAir 부착 소켓(블프 디폴트 `Fire_Air`). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|SkillSockets")
	FName DragonFireFlyAttachSocketName = FName(TEXT("Fire_Air"));

	/** 미티어 패턴 간격 — 레거시 타이머를 문자열 없이 처리. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 3", meta = (ClampMin = "0.05"))
	float MeteorSpawnPeriodSeconds = 2.5f;

	/** 미티어 타깃 거리 무작위 분산 (cm 단위 블프 `RandomFloatInRange` ±값). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 3", meta = (ClampMin = "0.0"))
	double MeteorAlongForwardRandomHalfRange = 2000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 2")
	TObjectPtr<AActor> CurFire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 3")
	int32 MeteorIndex = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 3")
	TObjectPtr<AActor> CurMeteor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 4")
	double MovedLength = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 4")
	TObjectPtr<AActor> SplineActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 4")
	TSubclassOf<AActor> SplineClass;

	/** /Game/.../EFlyState(FireAir) 과 동형의 저장 바이트. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 4")
	uint8 FireFlyState = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 4")
	TObjectPtr<UParticleSystem> FireAir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 4")
	TObjectPtr<AActor> CurFireAir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 5")
	uint8 FlyState = 0;

	/** 레거시 Skill_6 그래프 — `DoActionDatas[ThunderIndex]` 몽타주 선택(변수 이름은 BP 그대로). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 6")
	int32 ThunderIndex = 5;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "DragonWeapon|Settings",
		meta = (DisplayName = "Dragon", MultiLine = "true"))
	TObjectPtr<ACDragon> Dragon;

protected:
	/** 레거시 블프의 추가 SphereComponent 두 번째 근사물 — 내비 장애 레이어 블프튜와 동일하게 반경만 맞춘다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DragonWeapon|Components")
	TObjectPtr<USphereComponent> DragonWeaponNavOverlapExtra;

private:
	void Dragon_TickRotate(float DeltaSeconds);

	/** 레거시 이벤트 그래프 `Tick_FireFly` / `Tick_Fly`(Thunder) — 상태·스플라인 이동을 서버에서만 처리한다. */
	void Dragon_TickFireFlyBossGraph(float DeltaSeconds);

	void Dragon_TickFlyThunderBossGraph(float DeltaSeconds);

	/** 레거시 Begin Play — 태그 `Player` 로 첫 ACCommonCharacter 를 찾아 `Player` 변수에 넣음(있을 때만). */
	void Dragon_TryDiscoverPlayerTaggedActorIntoProperty();

	void Dragon_TryEnsureSplineActorSpawnedAtDragon(ACDragon* InDragon);

	static USplineComponent* Dragon_FindFirstSplineComponentOnActor(AActor* InActorCandidate);

	static void Dragon_ApplyBossFlyJumpAndLaunchBack(ACDragon* InDragonBoss);

	bool Dragon_StepFireFlyAlongSpline(ACDragon* InDragonBoss, float DeltaSeconds);

	void Dragon_ArriveFinishFireFlyLandSequence(ACDragon* InDragonBoss);

	void Dragon_ArriveFinishThunderFlyLandSequence(ACDragon* InDragonBoss);

	void Dragon_ClearFireFlyThunderLatentTimers();

	void Dragon_ResetFireFlyThunderRuntimeStateExceptSplineActor(bool bIncludeMovedLengthReset);

	static constexpr float DragonBossFireFlyWaitBeforeSetSeconds = 1.f;

	static constexpr float DragonBossFireFlyDelayBeforeFireFlyingFunctionSeconds = 1.f;

	static constexpr float DragonBossThunderFlyPauseBeforeLandSeconds = 1.5f;

	static constexpr float DragonBossSplineAlongSpeedMultiplierVsWalk = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragonWeapon|Skill 4", meta = (AllowPrivateAccess))
	FName DragonPlayerDiscoveryActorTagName = FName(TEXT("Player"));

	FTimerHandle DragonFireFlyWaitBeforeEnterFlyTimerHandle;

	FTimerHandle DragonFireFlyDelayFireFlyingFunctionTimerHandle;

	FTimerHandle DragonFlyThunderWaitBeforeLandTimerHandle;

	bool bDragonWeapon_FireFlyHasScheduledWaitStage = false;

	bool bDragonWeapon_FlyThunderLatentAwaitingLand = false;

	bool bDragonWeapon_FlyThunderPendingBurstJump = false;

	bool bDragonWeapon_FireFlyHasIssuedFlyingFunctionCallThisPhase = false;

	UFUNCTION()
	void Dragon_OnFireFly_WaitBeforeSplinePhaseTimerExpired();

	UFUNCTION()
	void Dragon_OnFireFly_DelayedFireFlyingFunctionTimerExpired();

	UFUNCTION()
	void Dragon_OnFlyThunder_WaitThenLandTimerExpired();

	/** 스킬 전환 시 이전 액터/타이머를 비우지 않으면 이펙트·중복 패킷 등이 남는다 — 서버 권위 단일 경로만 정리한다. */
	void Dragon_AbortSkillInFlight(ACDragon* InDragonResolved);

	void Dragon_ApplySkillPrelude(ACDragon* InDragonResolved) const;

	void Dragon_ApplySkillPostlude(ACDragon* InDragonResolved);

	int32 SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType InSkillType) const;

	float Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
		const FDoActionData* InMaybeRowOverride,
		int32 FallbackSkillWaitTimeIndex,
		float DefaultSecondsIfRowMissingMontage,
		UAnimMontage* InExplicitMontage,
		float InExplicitMontagePlayRate);

	void Skill_SchedulePhaseEnd(float InDelaySeconds);

	bool Skill_TryProcessVoidFunction(AActor* InTargetActor, const FName& InFunctionName) const;

	static void Skill_DestroyActorIfSpawnedSoft(AActor* InActorCandidate);

	void Dragon_RuntimeTeardownSpawnedSkillActors();

	/** 레거시 Skill_4 `FireFlying` — Neck 을 Fire_Air 로 붙였다가 스킬 종료 시 Fire_Ground 로 되돌린다. */
	void Dragon_RuntimeRestoreNeckAfterFireFlyIfNeeded(ACDragon* InDragonResolvedOrNullUsesResolveFallback);

	bool bDragonWeapon_FireFlyNeckWasReparentedForSkill4 = false;

	FTimerHandle DragonSkillPhaseEndTimerHandle;

	FTimerHandle DragonMeteorSpawnTimerHandle;

	UFUNCTION()
	void Dragon_OnSkillPhaseEndTimerExpired();

	UFUNCTION()
	void Dragon_OnMeteorSpawnTick();
};
