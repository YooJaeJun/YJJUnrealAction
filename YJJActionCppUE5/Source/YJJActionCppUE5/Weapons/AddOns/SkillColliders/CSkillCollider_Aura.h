#pragma once
#include "CoreMinimal.h"
#include "Weapons/AddOns/CSkillCollider.h"
#include "GameFramework/Actor.h"
#include "Weapons/CWeaponStructures.h"
#include "NiagaraDataInterfaceExport.h"
#include "CSkillCollider_Aura.generated.h"

class ACCommonCharacter;
class USceneComponent;
class UBoxComponent;

UCLASS()
class YJJACTIONCPPUE5_API ACSkillCollider_Aura :
	public ACSkillCollider,
	public INiagaraParticleCallbackHandler
{
	GENERATED_BODY()

public:	
	ACSkillCollider_Aura();

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

//public:
//	virtual void ReceiveParticleData_Implementation(
//		const TArray<FBasicParticleData>& ParticleData, 
//		UNiagaraSystem* NiagaraSystem);

private:
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UNiagaraComponent> Niagara;

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UBoxComponent> Box;


	UPROPERTY(EditDefaultsOnly, Category = "Damaged")
		FHitData HitData;

	UPROPERTY(EditDefaultsOnly, Category = "Damaged")
		float DamageInterval = 0.1f;

private:
	FTimerHandle TimerHandle;
};