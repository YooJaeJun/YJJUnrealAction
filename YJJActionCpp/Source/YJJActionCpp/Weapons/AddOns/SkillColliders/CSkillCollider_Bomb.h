#pragma once
#include "CoreMinimal.h"
#include "Weapons/AddOns/CSkillCollider.h"
#include "Weapons/CWeaponStructures.h"
#include "CSkillCollider_Bomb.generated.h"

class UCapsuleComponent;
class USphereComponent;
class UParticleSystemComponent;
class UNiagaraComponent;
class UProjectileMovementComponent;

UCLASS()
class YJJACTIONCPP_API ACSkillCollider_Bomb : public ACSkillCollider
{
	GENERATED_BODY()

public:
    ACSkillCollider_Bomb();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    void Bomb();

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
        UCapsuleComponent* Capsule;

    UPROPERTY(VisibleDefaultsOnly)
        UParticleSystemComponent* Particle;

    UPROPERTY(VisibleDefaultsOnly)
        UParticleSystemComponent* Trail;

    UPROPERTY(VisibleDefaultsOnly)
        USphereComponent* BombSphere;

    UPROPERTY(VisibleDefaultsOnly)
        UNiagaraComponent* BombParticle;

    UPROPERTY(VisibleDefaultsOnly)
        UProjectileMovementComponent* Projectile;


    UPROPERTY(EditDefaultsOnly, Category = "Damage")
        FHitData HitData;


    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        float Speed = 300;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        float BombRate = 2;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        float SpawnForwardLocationFactor = 10;

private:
    FTimerHandle TimerHandle;
    FVector Forward;
};
