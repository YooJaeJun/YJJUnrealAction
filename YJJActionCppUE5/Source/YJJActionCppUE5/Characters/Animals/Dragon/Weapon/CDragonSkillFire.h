#pragma once

#include "CoreMinimal.h"
#include "Characters/Animals/Dragon/Weapon/CDragonSkillMagic.h"
#include "CDragonSkillFire.generated.h"

class UParticleSystemComponent;
class UBoxComponent;
class UProjectileMovementComponent;

/**
 * 레거시 `/Game/Character/Animals/Dragon/Weapon/Fire/Skill_Dragon_Fire` (Skill_Dragon 파생).
 * 깨져 있던 Dragon·Neck·Character·BF_Helpers 노드를 ACDragon 파이프라인과 UCYJJBlueprintLibrary 로 정리했다.
 *
 * - Tick(Attached=true): Neck 소켓(SocketName) 변환으로 FireEffect 파티클 월드 변환 동기화(블루프린트처럼 겹침은 FireCollision에만 두고 파티클 충돌은 건드리지 않음).
 * - Fire(): FireCollision QueryOnly + 가시화, LoopSound 액터 위치 재생, Attached=true (`Skill_Dragon_Fire:Fire`).
 * - DragonFire_End() DisplayName "End": FireCollision NoCollision, FireEffect Emitter 비활성, Attached=false (`Skill_Dragon_Fire:End`).
 * - FireCollision BeginOverlap: 아군이면 무시, 아니면 HitDatas[FireIndex] 로 SendDamage; 맞았을 때는 ShootSound(루프는 Fire 시점 재생만).
 *
 * 블루프린트는 부모를 `ACSkillDragonFire` 로 바꾼 뒤, 중복 레거시 콜리전/그래프는 제거하고 컴포넌트·에셋만 유지하면 된다.
 */
UCLASS(
	Blueprintable,
	meta=(DisplayName = "Dragon Skill — Fire Breath", BlueprintDescription="Skill_Dragon_Fire 네이티브 버전입니다. 레거시 이벤트 그래프(Tick/Touch/데드바인드) 대신 본 클래스를 사용합니다."))
class YJJACTIONCPPUE5_API ACSkillDragonFire : public ACSkillDragon
{
	GENERATED_BODY()

public:
	ACSkillDragonFire();

	virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 블루프린트 사용자 함수 `Fire` 와 동일. */
	UFUNCTION(BlueprintCallable, Category = "Dragon|Skill|Fire")
	virtual void Fire();

	/** 블루프린트 사용자 함수 `End` 와 동일. C++ 이름은 UObject 모호함을 피하기 위해 분리했다. */
	UFUNCTION(BlueprintCallable, Category = "Dragon|Skill|Fire", meta = (DisplayName = "End"))
	virtual void DragonFire_End();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon|Skill|Fire|VFX")
	FName BreathEmitterToggleName = FName(TEXT("Fire"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Fire")
	TObjectPtr<USoundBase> ShootSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Fire")
	TObjectPtr<USoundBase> LoopSound = nullptr;

	/** 레거시 `Hit` 카테고리 — 이미 맞춘 타깃을 중복으로 맞히지 않는다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon|Skill|Fire|Hit")
	TArray<TObjectPtr<ACCommonCharacter>> Hitted;

	/** 레거시 `Socket` — Neck 기준 불 뿜는 소켓(Fire_Ground). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon|Skill|Fire")
	bool Attached = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon|Skill|Fire")
	FName SocketName = FName(TEXT("Fire_Ground"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Fire|Trace", meta = (ClampMin = "4.0"))
	float BetweenPrimitivesHitSphereRadius = 48.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Fire|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystemComponent> FireEffect = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Fire|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> FireCollision = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dragon|Skill|Fire|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement = nullptr;

protected:
	/** `Fire()` 에서 동일 처리 직후(루프 사운드 이후)·`Attached` 세팅 직전 호출된다. 공중 브레스는 여기에서 ShootSound 를 한 번 더 낸다. */
	virtual void DragonFire_OnAfterBaselineFireScratch();

	/** `DragonFire_End()` 에서 파티클 종료 처리를 한 곳으로 모았다. */
	virtual void DragonFire_ApplyEndVfxScratch();

	/** 틱에서 Neck 소켓으로 FireEffect 위치 맞춘 직후 호출된다. */
	virtual void DragonFire_OnAttachedBreathAfterTransformScratch();

private:
	UFUNCTION()
	void DragonFire_OnCharacterDeadDelegateScratch();

	void DragonFire_BindOverlapScratch();
	void DragonFire_UnbindOverlapScratch();
	void DragonFire_BindDeadDelegateScratch();
	void DragonFire_UnbindDeadDelegateScratch();

	void DragonFire_UpdateAttachedBreathScratch();
	void DragonFire_OnFireOverlapScratch(
		UPrimitiveComponent* OverlappedComponentScratch,
		AActor* OtherActorScratch,
		UPrimitiveComponent* OtherCompScratch,
		int32 OtherBodyIndexScratch,
		bool bFromSweepScratch,
		const FHitResult& SweepResultScratch);

	static bool DragonFire_ListContainsScratch(
		const TArray<TObjectPtr<ACCommonCharacter>>& ListScratch,
		ACCommonCharacter* CandidateScratch);

	bool DragonFire_HasAuthorityForDamageScratch() const;
	static void DragonFire_CopyIgnoresScratch(
		const TArray<TObjectPtr<AActor>>& SourceActorsToIgnoreScratch,
		AActor* ExplicitCharacterScratch,
		TArray<AActor*>& OutIgnoresScratch);
};
