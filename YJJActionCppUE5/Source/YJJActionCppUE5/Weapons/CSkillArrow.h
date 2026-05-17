#pragma once

#include "CoreMinimal.h"
#include "Weapons/CSkillWeapon.h"

#include "CSkillArrow.generated.h"

// UObject* 는 블루프린트에서 형 변환으로 Arrow 를 받을 수 있으며, 동적 디스패처는 generated.h 뒤(엔진 패턴과 동일)에 둬야 MSVC 가 전개한 래퍼를 올바르게 파싱한다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArrowDestroiedSignature, UObject*, InArrow);

class USphereComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;
class UProjectileMovementComponent;

/** BP Skill_Arrow 과 동형: DefaultSceneRoot → Sphere(반경 20, Nav obstacle) → StaticMesh SM_ElvenArrow + Trail P_Trail_Trace(스케일 2×2×2, 비표시), ProjectileMovement(8000 중력 0 비자동 활성화). Shoot 시 트레일 표시. OnArrowDestroied(InArrow UObject). */
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACSkillArrow : public ACSkillWeapon{
	GENERATED_BODY()

public:
	ACSkillArrow();

protected:
	// BP: ReceiveEndPlay → Super → OnArrowDestroied.Broadcast(Self) 와 동일 순서.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Arrow|Events")
	FOnArrowDestroiedSignature OnArrowDestroied;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow|Components")
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow|Components")
	TObjectPtr<UStaticMeshComponent> ArrowStaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow|Components")
	TObjectPtr<UParticleSystemComponent> Trail;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/**
	 * 레거시 BP Skill_Arrow::Shoot 와 호환되는 발사 디스패치.
	 * 디폴트는 Projectile 에 방향·InitialSpeed 로 속도 설정 — 블루프린트에서 동일 이름으로 재정의 가능.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Arrow")
	void Shoot(const FVector& InDirection);
	virtual void Shoot_Implementation(const FVector& InDirection);
};
