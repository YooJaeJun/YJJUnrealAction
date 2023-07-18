#pragma once
#include "CoreMinimal.h"
#include "Weapons/AddOns/CSkillCollider.h"
#include "Weapons/CWeaponStructures.h"
#include "CSkillCollider_Throw.generated.h"

class UCapsuleComponent;
class UParticleSystemComponent;
class UProjectileMovementComponent;

UCLASS()
class YJJACTIONCPP_API ACSkillCollider_Throw : public ACSkillCollider
{
	GENERATED_BODY()

public:
    ACSkillCollider_Throw();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

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

    UFUNCTION()
        void SendDamage();

private:
    UPROPERTY(VisibleDefaultsOnly)
        UCapsuleComponent* Capsule;

    UPROPERTY(VisibleDefaultsOnly)
        UParticleSystemComponent* Particle;

    UPROPERTY(VisibleDefaultsOnly)
        UProjectileMovementComponent* Projectile;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
        FHitData HitData;


    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        float Speed = 300;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        float DamageInteval = 0.1f;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        float SpawnForwardLocationFactor = 10;

private:
    FTimerHandle TimerHandle;
    FVector Forward;
};
