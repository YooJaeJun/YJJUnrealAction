#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CWeapon.generated.h"

class UAnimMontage;
class UParticleSystem;
class USoundWave;
class UMatineeCameraShake;
class USceneComponent;
class USkeletalMeshComponent;
class ACCharacter;

UCLASS()
class YJJACTIONCPP_API ACWeapon : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Equip")
		FName HolsterSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Equip")
		UAnimMontage* EquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Equip")
		float EquipMontage_PlayRate;

	UPROPERTY(EditDefaultsOnly, Category = "Act")
		UParticleSystem* ActionParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Act")
		USoundWave* ActionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Act")
		TSubclassOf<UMatineeCameraShake> CameraShakeClass;

	UPROPERTY(VisibleAnywhere)
		USkeletalMeshComponent* Mesh;

private:
	UPROPERTY(VisibleAnywhere)
		USceneComponent* Root;

	FORCEINLINE bool IsActing() { return bActing; }

public:	
	ACWeapon();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

protected:
	TWeakObjectPtr<ACCharacter> Owner;

private:
	bool bEquipping;
	bool bActing;
	bool bFiring;
};