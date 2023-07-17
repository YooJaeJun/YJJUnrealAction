#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CArrow.generated.h"

class UCapsuleComponent;
class UProjectileMovementComponent;
class ACCommonCharacter;
class ACArrow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FArrowHit, 
	AActor*, InCauser, 
	ACCommonCharacter*, InOtherCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FArrowEndPlay, 
	ACArrow*, InDestroyer);

UCLASS()
class YJJACTIONCPPUE5_API ACArrow : public AActor
{
	GENERATED_BODY()

public:	
	ACArrow();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void Shoot(const FVector& InForward) const;

private:
	UFUNCTION()
		void OnComponentHit(
			UPrimitiveComponent* HitComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			FVector NormalImpulse,
			const FHitResult& Hit);

public:
	FORCEINLINE void AddIgnoreActor(TWeakObjectPtr<AActor> InActor) { Ignores.Add(InActor); }

private:
	UPROPERTY(VisibleDefaultsOnly)
		TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleDefaultsOnly)
		TObjectPtr<UProjectileMovementComponent> Projectile;

	UPROPERTY(EditDefaultsOnly, Category = "LifeSpan")
		float LifeSpanAfterCollision = 2.0f;

public:
	FArrowHit OnArrowHit;
	FArrowEndPlay OnArrowEndPlay;

private:
	TArray<TWeakObjectPtr<AActor>> Ignores;
};