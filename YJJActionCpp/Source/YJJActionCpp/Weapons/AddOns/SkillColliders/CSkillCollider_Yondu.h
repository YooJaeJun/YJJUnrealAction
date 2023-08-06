#pragma once
#include "CoreMinimal.h"
#include "Weapons/AddOns/CSkillCollider.h"
#include "Weapons/CWeaponStructures.h"
#include "Commons/CEnums.h"
#include "CSkillCollider_Yondu.generated.h"

class UCapsuleComponent;
class UParticleSystemComponent;
class UParticleSystem;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class USplineComponent;

UCLASS()
class YJJACTIONCPP_API ACSkillCollider_Yondu : public ACSkillCollider
{
	GENERATED_BODY()

public:
    ACSkillCollider_Yondu();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    void Shoot();
    void ComeBack();
    void Ready();

private:
    bool Tick_Move();
    void SetTargets();
    void ResetTargets();

public:
    FORCEINLINE void SetDefaultTransform(const FTransform& InTransform) { DefaultTransform = InTransform; }

private:
    UFUNCTION()
        void OnComponentBeginOverlap(
            UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex,
            bool bFromSweep,
            const FHitResult& SweepResult);

    UFUNCTION()
        void OnComponentEndOverlap(
            UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex);

private:
    UPROPERTY(VisibleDefaultsOnly)
        USceneComponent* Root;

    UPROPERTY(VisibleDefaultsOnly)
        USceneComponent* ArrowRoot;

    UPROPERTY(VisibleDefaultsOnly)
        UCapsuleComponent* Capsule;

    UPROPERTY(VisibleDefaultsOnly)
        UParticleSystemComponent* ParticleComp;

    UPROPERTY(VisibleDefaultsOnly)
        UParticleSystem* ParticleAsset;

    UPROPERTY(EditAnywhere)
        UNiagaraComponent* TrailComp;

    UPROPERTY(EditAnywhere)
        UNiagaraSystem* TrailAsset;

    UPROPERTY(EditAnywhere, Category = "Spline")
        USplineComponent* SplineComp;

    UPROPERTY(EditDefaultsOnly, Category = "Spline")
        TEnumAsByte<CEProjectileState> ProjectileState = CEProjectileState::None;


    UPROPERTY(EditDefaultsOnly, Category = "Damage")
        FHitData HitData;


    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        FTransform DefaultTransform;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        float TraceRadius = 1500;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        float EndTime = 8;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        float Speed = 2000;

private:
    TWeakObjectPtr<ACCommonCharacter> Owner;
    TArray<TWeakObjectPtr<ACCommonCharacter>> Targets;
    float MovedDistance = 0;
};
