#pragma once

#include "CoreMinimal.h"
#include "Characters/Animals/Dragon/Weapon/CDragonSkillMagic.h"
#include "Camera/CameraShakeBase.h"
#include "CDragonSkillMeteor.generated.h"

class ACCommonCharacter;
class UNiagaraSystem;
class USoundBase;
class UParticleSystemComponent;
class UProjectileMovementComponent;
class USceneComponent;
class USphereComponent;

/**
 * 레거시 `/Game/Character/Animals/Dragon/Weapon/Meteor/Skill_Dragon_Meteor`.
 *
 * 무기(ACDragonWeapon)가 스폰 후 `Shoot` 호출하고, 종료 시 `End` 다음 Destroy 한다.
 *
 * 블프 덤프 근거:
 * - Shoot: ProjectileMovement 활성화(레거시), CurScale`(1,1,1)`, ShootSound 현재 위치 재생.
 * - Tick(Gate): 유효 루트·DefaultSceneRoot가 있으면 Sphere 박스 크기 증폭 후 `CurScale *= 1.005`, 루트 위치는 Dest로 `VInterpTo`(Δ=월드Δ, 속도 1).
 * - Sphere overlap: Owner 유효, Other!=Owner 후 Character / Landscape / StaticMesh 이면 `Bomb()` 호출.
 * - 깨졌던 BeginPlay( Dragon·IsDead ) 대신 피격 주체(Character) 의 `OnIsDead` 에서 권위에서만 `Destroy` 한다.
 * - Bomb: BombSphere Collision QueryAndPhysics, Niagara, ProjectileGravityScale=0, Sphere off, 폭발 연출(전용 서버에서는 생략),
 *   `SphereTraceMultiForObjects`(반경 `BombDamageOverlapRadiusUU`) + `ActorsToIgnore` 후 비아군 `LaunchCharacter`·`SendDamage`,
 *   끝에서 권위 액터만 `Destroy`(무기·`CurMeteor` dangling 방지 위해 `EndPlay`에서 무기 레퍼런스 초기화).
 *
 * ProjectileMovement 와 매 틱 Dest 보간 이 동시에 켜지면 결과가 블프 정의모호하다. 본 구현에서는 **항상 Dest 로만 보간** 이동시키며,
 * Shoot 에서 ProjectileMovement 재생 상태만 블프와 이름을 맞춘다(실제 속도에는 기여하지 않는다고 가정 가능).
 *
 * 블프는 부모를 `ACSkillDragonMeteor` 로 바꾸고 레거시 합성(Tick/FireOverlap/바인드) 은 비우거나 중복 변수를 제거한다.
 */
UCLASS(
	Blueprintable,
	meta=(
		DisplayName = "Dragon Skill — Meteor",
		BlueprintDescription = "Skill_Dragon_Meteor 네이티브 버전입니다."))
class YJJACTIONCPPUE5_API ACSkillDragonMeteor : public ACSkillDragon
{
	GENERATED_BODY()

public:
	ACSkillDragonMeteor();

	virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 무기 등에서 이름으로 호출된다 — 시그니처·이름 변경 금지. */
	UFUNCTION(BlueprintCallable, Category = "Dragon|Skill|Meteor")
	void Shoot();

	/** 무기 티어다운이 `Skill_TryProcessVoidFunction(_, End)` 로 조회함 — 함수명이 반드시 `End` 여야 한다. */
	UFUNCTION(BlueprintCallable, Category = "Dragon|Skill|Meteor")
	void End();

	/** 격발 측 Sphere overlap 과 동일 이름 — 블프 오버라이드·외부 디스패치 대비 노출 유지. */
	UFUNCTION(BlueprintCallable, Category = "Dragon|Skill|Meteor")
	void Bomb();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Audio")
	TObjectPtr<USoundBase> BombSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Audio")
	TObjectPtr<USoundBase> ShootSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|VFX")
	TObjectPtr<UNiagaraSystem> BombEffect;

	/** 레거시 `Hit`: 폭발 AOE 또는 직통 피격 시 중복 제거. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon|Skill|Meteor|Hit")
	TArray<TObjectPtr<ACCommonCharacter>> Hitted;

	/** 레거시 `Dest`: 미설정이면 Shoot 직후 전방 피벗으로 채운다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon|Skill|Meteor")
	FVector Dest = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon|Skill|Meteor")
	FVector CurScale = FVector::OneVector;

	/** 레거시 `VInterpTo` InterpSpeed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Motion", meta = (ClampMin = "0.01"))
	float MoveInterpSpeed = 1.0f;

	/** 레거시 `Multiply_VectorFloat` 배율(기본 `1.005`). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Motion", meta = (ClampMin = "1.0"))
	float GrowScaleMultiplierPerTick = 1.005f;

	/** 레거시 `BombSphere`(표시 참고)·블프 컴포넌트 이름 정합. 피격 쿼리 반경은 `BombDamageOverlapRadiusUU` 가 우선된다. */
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Dragon|Skill|Meteor|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> BombSphere;

	/** 레거시 투사·겹침 본체. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystemComponent> MeteorParticleMain;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystemComponent> MeteorParticleTrail;

	/** 레거시 `ProjectileMovementComponent` 초기설정 블프와 이름 정합(`Skill_Dragon` 의 Get 노드). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Bomb", meta = (ClampMin = "64.0"))
	float BombDamageOverlapRadiusUU = 1000.0f;

	/** 레거시 `Bomb` → `PlayWorldCameraShake`(CS_Bomb). 에셋이 없으면 셰이크를 생략한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Bomb")
	TSubclassOf<UCameraShakeBase> BombCameraShakeClass;

	/** 블프 `PlayWorldCameraShake` InnerRadius 디폴트 0에 맞춤. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Bomb", meta = (ClampMin = "0.0"))
	float BombCameraShakeInnerRadiusUU = 0.f;

	/** 블프 `OuterRadius` 4000. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Bomb", meta = (ClampMin = "1.0"))
	float BombCameraShakeOuterRadiusUU = 4000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Bomb", meta = (ClampMin = "0.0"))
	float BombCameraShakeFalloff = 1.f;

	/** 블프 `LaunchCharacter`: `Normalize2D(Meteor - Victim) * float`. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Bomb", meta = (ClampMin = "0.0"))
	float BombLaunchKnockbackSpeedUU = 500.f;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Meteor|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> MeteorVisualPivot;

	UFUNCTION()
	void DragonMeteor_OnCharacterDeadDelegateScratch();

	void DragonMeteor_BindOverlapScratch();
	void DragonMeteor_UnbindOverlapScratch();
	void DragonMeteor_BindDeadDelegateScratch();
	void DragonMeteor_UnbindDeadDelegateScratch();

	bool DragonMeteor_HasAuthorityForDamageScratch() const;
	static bool DragonMeteor_ListContainsScratch(
		const TArray<TObjectPtr<ACCommonCharacter>>& ListScratch,
		ACCommonCharacter* CandidateScratch);

	void DragonMeteor_OnProjectileSphereOverlapScratch(
		UPrimitiveComponent* OverlappedComponentScratch,
		AActor* OtherActorScratch,
		UPrimitiveComponent* OtherCompScratch,
		int32 OtherBodyIndexScratch,
		bool bFromSweepScratch,
		const FHitResult& SweepResultScratch);

	void DragonMeteor_TryBombFromOverlappedActorScratch(AActor* OtherActorScratch);

	void DragonMeteor_ApplyBombDamageAoEScratch(const FVector& ExplosionOriginScratch);

	bool bMeteorBombAlreadyTriggeredScratch = false;
	bool bMotionEnabledScratch = false;
};
