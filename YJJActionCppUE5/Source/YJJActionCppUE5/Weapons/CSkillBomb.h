#pragma once

#include "Weapons/CSkillWeapon.h"
#include "Camera/CameraShakeBase.h"

#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

#include "CSkillBomb.generated.h"

class UParticleSystemComponent;
class UProjectileMovementComponent;
class USceneComponent;
class USphereComponent;

/**
 * 레거시 `/Game/Magics/Bomb/Skill_Bomb` 블프 — `Skill_Magic` 상속 노드 포팅(`Shoot`, `Bomb`, `Destroy`).
 * `ACMagicBombSkillContext` 가 스폰 후 `Shoot`(방향) 호출한다.
 *
 * 레거시 동작 근거(덤프):
 * - Shoot: `ActorsToIgnore` 에 Self·Character(+ 유효 시 CurInteractingActor) 누적, 퓨즈 `K2_SetTimer`("Bomb").
 * - Shoot 동시: ProjectileMovement 속도(`InDirection`*`InitialSpeed`), 컴포넌트 활성.
 * - Bomb(ExecutionSequence 순): `BombSphere` QueryAndPhysics, 나이아라 폭발, `ProjectileGravityScale=0`,
 *   `BombSound`/에픽센터 `PlayWorldCameraShake`(CS_Bomb, Outer≈2000), 이후 Destroy 를 추가 티이머로 스케줄.
 * - 병렬 의도였던 SphereTrace 결과를 순차 재현할 때 피격: BlockingHit 만, 피격자를 캐릭터로 캐스팅 후
 *   시전자 그룹이 같으면 스킵, 아니면 `Hitted` 중복 검사 후 `LaunchCharacter`(평면 밀림*500)·`Magic.HitCommonDatas[0]` 피격.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Skill Bomb"))
class YJJACTIONCPPUE5_API ACSkillBomb : public ACSkillMagic
{
	GENERATED_BODY()

public:
	ACSkillBomb();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Magic_Bomb 가 스폰 직후 호출 — 레거시 CustomEvent `Shoot`. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Skill|Bomb")
	void Shoot(FVector const& InDirection);
	virtual void Shoot_Implementation(FVector const& InDirection);

	/** 퓨즈 만료 또는 블프/디버그 디스패치 — 레거시 `Bomb` 함수. */
	UFUNCTION(BlueprintCallable, Category = "Skill|Bomb")
	void Bomb();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Bomb|Legacy", meta = (DisplayName = "Max Mass"))
	double MaxMass = 300.0;

	/** 레거시 `SphereTraceMulti` 반경(cm). 블프 `Radius` 디폴트 1000. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Bomb|Damage", meta = (ClampMin = "64.0"))
	float BombSphereTraceRadiusUU = 1000.0f;

	/** 레거시 `Multiply_VectorFloat` 블프 핀 디폴트 500(knockback 속도 크기·cm/s 근사). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Bomb|Damage", meta = (ClampMin = "0.0"))
	float BombLaunchKnockbackSpeedUU = 500.f;

	/** 레거시 `K2_SetTimer`("Bomb") — 발사 후 폭발 지연 시간(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Bomb", meta = (ClampMin = "0.0"))
	float BombFuseDelaySeconds = 1.0f;

	/** 레거시 `K2_SetTimer`("Destroy") — Bomb 직후 액터 정리까지 지연(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Bomb", meta = (ClampMin = "0.0"))
	float BombPostExplosionDestroyDelaySeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Bomb|VFX|SFX")
	TObjectPtr<UNiagaraSystem> BombEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Bomb|VFX|SFX")
	TObjectPtr<USoundBase> BombSound;

	/** 레거시 `PlayWorldCameraShake` — `/Game/Magics/Bomb/CS_Bomb` 디폴트는 ctor 에서 채운다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Bomb|CameraShake")
	TSubclassOf<UCameraShakeBase> BombCameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Bomb|CameraShake", meta = (ClampMin = "0.0"))
	float BombCameraShakeInnerRadiusUU = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Bomb|CameraShake", meta = (ClampMin = "1.0"))
	float BombCameraShakeOuterRadiusUU = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Bomb|CameraShake", meta = (ClampMin = "0.0"))
	float BombCameraShakeFalloff = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Bomb|Overlap")
	TArray<TObjectPtr<AActor>> ActorsToIgnore;

	/** 레거시 `Hitted` — 같은 폭발 내 동일 타겟·`Magic.HitCommonDatas[0]` 경로 피격 1회. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Bomb|Damage")
	TArray<TObjectPtr<ACCommonCharacter>> Hitted;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Skill|Bomb|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> BombPivot;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Skill|Bomb|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> BombSphere;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Skill|Bomb|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystemComponent> ProjectileFireballParticle;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Skill|Bomb|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystemComponent> ProjectileTrailParticle;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Skill|Bomb|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

private:
	static void SkillBombTryApplyProjectileTrailCascadeDefaults(UParticleSystemComponent* TrailParticleScratch);

	void SkillBomb_RebuildActorsToIgnoreScratch();
	void SkillBomb_ClearFuseTimersScratch();
	static bool SkillBomb_HittedAlreadyContainsScratch(
		const TArray<TObjectPtr<ACCommonCharacter>>& HittedScratch, ACCommonCharacter* CandidateScratch);

	void SkillBomb_TimerCallback_FuseBomb();
	void SkillBomb_TimerCallback_DestroyAfterBomb();

	bool bBombExecutedScratch = false;

	FTimerHandle FuseToBombTimerHandle;
	FTimerHandle DestroyAfterBombTimerHandle;

	void Bomb_ApplyDamageAoEScratch(UWorld* WorldScratch, const FVector& ExplosionOriginScratch);
	bool Bomb_ShouldRunCosmeticsScratch() const;
	bool Bomb_HasAuthorityForDamageScratch() const;
};
