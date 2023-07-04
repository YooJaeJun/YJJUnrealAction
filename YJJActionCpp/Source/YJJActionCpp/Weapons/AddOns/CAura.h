#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapons/CWeaponStructures.h"
#include "NiagaraDataInterfaceExport.h"
#include "CAura.generated.h"

class ACCommonCharacter;
class USceneComponent;
class UBoxComponent;

UCLASS()
class YJJACTIONCPP_API ACAura :
	public AActor,
	public INiagaraParticleCallbackHandler
{
	GENERATED_BODY()

public:	
	ACAura();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
		void OnSystemFinished(UNiagaraComponent* PSystem);

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

public:
	virtual void ReceiveParticleData_Implementation(
		const TArray<FBasicParticleData>& ParticleData, 
		UNiagaraSystem* NiagaraSystem) override;

private:
	UPROPERTY(VisibleAnywhere)
		USceneComponent* Root;

	UPROPERTY(VisibleAnywhere)
		UNiagaraComponent* Niagara;

	UPROPERTY(VisibleAnywhere)
		UBoxComponent* Box;


	UPROPERTY(EditDefaultsOnly, Category = "Damaged")
		FHitData HitData;

	UPROPERTY(EditDefaultsOnly, Category = "Damaged")
		float DamageInterval = 0.1f;

private:
	TArray<TWeakObjectPtr<ACCommonCharacter>> Hitted;
	FTimerHandle TimerHandle;
};
