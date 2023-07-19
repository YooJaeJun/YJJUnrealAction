#pragma once
#include "CoreMinimal.h"
#include "Weapons/AddOns/CSkillCollider.h"
#include "Weapons/CWeaponStructures.h"
#include "CSkillCollider_Bomb.generated.h"

class UCapsuleComponent;
class USphereComponent;
class UParticleSystemComponent;
class UFXSystemAsset;
class UProjectileMovementComponent;
class USoundBase;
class UMatineeCameraShake;

UCLASS()
class YJJACTIONCPP_API ACSkillCollider_Bomb : public ACSkillCollider
{
	GENERATED_BODY()

public:
    ACSkillCollider_Bomb();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    void Throw() const;

private:
    void Bomb() const;

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
        void OnBombComponentBeginOverlap(
            UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex,
            bool bFromSweep,
            const FHitResult& SweepResult);

    UFUNCTION()
        void OnBombComponentEndOverlap(
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
        UProjectileMovementComponent* Projectile;

    UPROPERTY(EditAnywhere, Category = "Bomb")
        UFXSystemAsset* BombEffect;

    UPROPERTY(EditAnywhere, Category = "Bomb")
        USoundBase* BombSound;

    UPROPERTY(EditAnywhere, Category = "Bomb")
        TSubclassOf<UMatineeCameraShake> BombCameraShake;


    UPROPERTY(EditDefaultsOnly, Category = "Damage")
        FHitData HitData;


    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        float SpawnForwardLocationFactor = 10;


    UPROPERTY(EditDefaultsOnly, Category = "Bomb")
        float Speed = 300;

    UPROPERTY(EditDefaultsOnly, Category = "Bomb")
        float BombRate = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Bomb")
        float BombLaunch = 1000;

private:
    FTimerHandle TimerHandle;
    FVector Direction;
};
