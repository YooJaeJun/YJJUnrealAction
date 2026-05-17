#pragma once

#include "Weapons/CSkillWeapon.h"

#include "Components/PrimitiveComponent.h"

#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"

#include "Characters/CCommonCharacter.h"

#include "CSkillYondu.generated.h"
class UCapsuleComponent;
class UNiagaraComponent;
class UParticleSystemComponent;
class USplineComponent;

/**
 * 레거시 `/Game/Magics/Yondu/Skill_Yondu`(Skill_Magic) 포팅.
 * - `EProjectileState` UDS 항목 순서(바이트 0~3)와 동일: None / Ready / Shooting / Back
 * - 틱: 부모 `ReceiveTick` 후 상태 스위치 — Ready 는 `ArrotRoot` 를 `MagicDefaultTransform`(월드)로 고정, Shooting 은 스플라인 거리·이동 순서가 블프와 동일(샘플 → `MovedLength` 가산), 끝에서 `Shoot` 재호출
 * - `LifeTimeSeconds` 경과 시 Back → `ArrotRoot` 를 기본 자세로 이징(블프 `MoveComponentTo` 1s·EaseIn) → Ready
 * - 캡슐 `OnComponentBeginOverlap`: ignore 목록 제외, 아군 제외, `Magic.HitCommonDatas[0]` 로 `SetDamagedInfo` + `SendDamage`
 *
 * 레거시 `BF_Helpers` 노드 깨짐은 C++ `AreCharactersSameGroup`·직접 중복 검사로 대체한다.
 */
UENUM(BlueprintType)
enum class EProjectileState_Yondu : uint8
{
	/** UDS `NewEnumerator0` — 표시명 None. */
	None UMETA(DisplayName = "None"),
	/** UDS `NewEnumerator2`. */
	Ready UMETA(DisplayName = "Ready"),
	/** UDS `NewEnumerator3`. */
	Shooting UMETA(DisplayName = "Shooting"),
	/** UDS `NewEnumerator4`. */
	Back UMETA(DisplayName = "Back"),
};

UCLASS(Blueprintable, meta = (DisplayName = "Skill Yondu"))
class YJJACTIONCPPUE5_API ACSkillYondu : public ACSkillMagic
{
	GENERATED_BODY()

public:
	ACSkillYondu();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	/** 레거시 CustomEvent `Shoot` — 매개변수 없음(Yondu). */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Skill|Yondu")
	void Shoot();

	virtual void Shoot_Implementation();

	/** 레거시 BPVar 트레이스 반경 디폴트 1000. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Yondu|Trace", meta = (ClampMin = "1.0"))
	float SkillYonduTraceRadiusUU = 1000.f;

	/** 레거시 `Multiply_VectorFloat` 벡터 핀 디폴트 100(cm) — 타깃 로컬 진행 거리 근사. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Yondu|Spline", meta = (ClampMin = "0.0"))
	float SkillYonduSplineAlongVictimUU = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Yondu", meta = (MultiLine = "true"))
	TArray<TObjectPtr<AActor>> ActorsToIgnore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Yondu")
	TArray<TObjectPtr<ACCommonCharacter>> Targets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Yondu")
	int32 CurIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Yondu")
	double MovedLength = 0.0;

	/** 레거시 BP 디폴트 약 3000 cm/s 근거. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Yondu", meta = (ClampMin = "0.0"))
	float SkillSpeed = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Yondu")
	FTransform MagicDefaultTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Yondu")
	EProjectileState_Yondu ProjectileState = EProjectileState_Yondu::Ready;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Yondu", meta = (ClampMin = "0.05"))
	float LifeTimeSeconds = 10.f;

	/** 블프 `MoveComponentTo` OverTime 디폴트 1.0. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Yondu", meta = (ClampMin = "0.05"))
	float MoveBackSeconds = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Yondu|VFX")
	TObjectPtr<UNiagaraSystem> YonduNiagaraSystemAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Yondu|VFX")
	TObjectPtr<UParticleSystem> YonduCascadeProjectileAsset;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Skill|Yondu|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> ArrotRoot;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Skill|Yondu|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Skill|Yondu|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Skill|Yondu|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> YonduNiagara;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Skill|Yondu|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystemComponent> YonduCascade;

protected:
	/** 활주 시작 이펙트 — 클라 전용; Dedicated 리슨·서버는 `SkillYondu_ShouldRunCosmeticsScratch` 로 스킵. */
	UFUNCTION(NetMulticast, Reliable)
	void SkillYondu_MulticastStartFlightScratch();

	UFUNCTION()
	void SkillYondu_OnCapsuleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

private:
	/** 레거시 `RandomFloatInRange` Z 오프셋 최댓값(cm). 틱·트레이스라 스팸 방지 위해 상수 고정 주석 참고. */
	static constexpr float SkillYonduRandomZMaxUUScratch = 100.f;

	void SkillYondu_TimeoutCallback();

	FTimerHandle SkillYondu_LifeTimeTimerHandleScratch;

	double SkillYondu_MoveBackElapsedScratch = 0.0;
	FTransform SkillYondu_MoveBackStartWorldScratch = FTransform::Identity;

	void SkillYondu_RebuildActorsToIgnoreScratch();
	bool SkillYondu_ShouldRunCosmeticsScratch() const;
	void SkillYondu_BindSoftAssetsIfUnsetScratch();
	void SkillYondu_StartLifeTimerScratch();
	void SkillYondu_UpdateCapsuleHitCollisionScratch();

	void SkillYondu_TickReadyScratch(float DeltaSeconds);
	void SkillYondu_TickShootingScratch(float DeltaSeconds);
	void SkillYondu_TickBackScratch(float DeltaSeconds);

	/** 레거시 `IsAlive`/StatComp 깨진 BP 노드 대체 — `CurHp<=0` 이 아닐 때만 true. 틱·트레이스가 아니라 맞을 때만 호출. */
	bool SkillYondu_TargetAppearsLivingScratch(ACCommonCharacter* Candidate) const;

	/** `TryAddUniqueCommonCharacter` 와 동일 의미 TObjectPtr 버전으로 유지 보수를 한곳에 둠. */
	static bool SkillYondu_TryAddUniqueVictimScratch(TArray<TObjectPtr<ACCommonCharacter>>& InOutTargets, ACCommonCharacter* Candidate);
};
