#pragma once

#include "Weapons/CSkillWeapon.h"

#include "Components/SphereComponent.h"
#include "Engine/EngineTypes.h"

#include "CSkillFireBall.generated.h"



class UPrimitiveComponent;
class UParticleSystem;
class UParticleSystemComponent;
class UProjectileMovementComponent;

/**
 * 레거시 `/Game/Magics/FireBall/Skill_FireBall`(Skill_Magic 근거).
 * - Sphere(반경 ~55)·스케일 0.7·`OverlapAll`(덤프), `ProjectileMovement`(InitialSpeed 1500, 중력 0, 기본 비활성)
 * - `Shoot(InDirection)` : 방향 정규화 후 `Velocity = 방향 × InitialSpeed`, 투사체 이동·파티클 활성
 * - Sphere `OnComponentBeginOverlap`: 캐릭터가 아니거나 캐스터면 무시 또는 정리만, 블프 `IsSameGroup` 대체 후 아군이면 피격 없이 소멸, 적이면 `Magic.HitCommonDatas[0]` + `Destroy`
 */
UCLASS(Blueprintable, meta = (DisplayName = "Skill Fire Ball"))
class YJJACTIONCPPUE5_API ACSkillFireBall : public ACSkillMagic
{
	GENERATED_BODY()

public:
	ACSkillFireBall();

	virtual void BeginPlay() override;

	/** 레거시 CustomEvent `Shoot` — 블프 `InDirection`(월드)·`ProjectileMovement.Velocity` 로 반영된다. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Skill|FireBall")
	void Shoot(FVector const& InDirection);

	virtual void Shoot_Implementation(FVector const& InDirection);

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Skill|FireBall|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Skill|FireBall|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystemComponent> FireParticle;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Skill|FireBall|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** 블프 `ProjectileMovement InitialSpeed`(1500). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|FireBall", meta = (ClampMin = "1.0"))
	float FireBallInitialSpeedScratch = 1500.f;

	/** 블프 `P_ky_shot_fire`; 비어 있으면 ctor·BeginPlay 에서 소프트 로드 경로 채운다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|FireBall|VFX")
	TObjectPtr<UParticleSystem> FireBallParticleAsset;

protected:
	UFUNCTION()
	void SkillFireBall_OnSphereBeginOverlapScratch(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		FHitResult const& SweepResult);

private:
	static const TCHAR* const SkillFireBallParticleSoftPathScratch;

	bool bSkillFireBallOverlapConsumedScratch = false;

	bool SkillFireBall_ShouldRunCosmeticsScratch() const;
	void SkillFireBall_BindParticleIfUnsetScratch();
	bool SkillFireBall_TargetLivingScratch(ACCommonCharacter* Candidate) const;
};
