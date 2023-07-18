#pragma once
#include "CoreMinimal.h"
#include "Weapons/AddOns/CSkillCollider.h"
#include "Weapons/CWeaponStructures.h"
#include "CSkillCollider_Rotate.generated.h"

class UCapsuleComponent;
class UParticleSystemComponent;

UCLASS()
class YJJACTIONCPP_API ACSkillCollider_Rotate : public ACSkillCollider
{
	GENERATED_BODY()

public:
    ACSkillCollider_Rotate();

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


    UPROPERTY(EditDefaultsOnly, Category = "Damage")
        FHitData HitData;


    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        float Speed = 300;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        float Distance = 150;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        bool bNegative;

    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
        float DamageInteval = 0.1f;

private:
    float Angle;
    FTimerHandle TimerHandle;
};