#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAttachment.generated.h"

class ACCommonCharacter;
class USceneComponent;
class UShapeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAttachmentBeginCollision);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAttachmentEndCollision);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAttachmentBeginOverlap,
	ACCommonCharacter*, InAttacker, 
		AActor*, InAttackCauser,
		ACCommonCharacter*, InOther);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAttachmentEndOverlap,
	ACCommonCharacter*, InAttacker,
		ACCommonCharacter*, InOther);

UCLASS()
class YJJACTIONCPPUE5_API ACAttachment : public AActor
{
	GENERATED_BODY()
	
public:	
	ACAttachment();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintNativeEvent)
		void OnBeginEquip();
	virtual void OnBeginEquip_Implementation() { }

	UFUNCTION(BlueprintNativeEvent)
		void OnUnequip();
	virtual void OnUnequip_Implementation() {}

protected:
	UFUNCTION(BlueprintCallable, Category = "Attach")
		void AttachTo(FName InSocketName);

	UFUNCTION(BlueprintCallable, Category = "Attach")
		void AttachToCollision(FName InCollisionName);

	UFUNCTION(BlueprintCallable, Category = "Detach")
		void DetachTo(FName InSocketName);

	UFUNCTION(BlueprintCallable, Category = "Detach")
		void DetachToCollision(FName InCollisionName);

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
	void OnCollisions();
	void OffCollisions();

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		TObjectPtr<USceneComponent> Root;

	UPROPERTY(BlueprintReadOnly, Category = "Game")
		TArray<TObjectPtr<UShapeComponent>> Collisions;

public:
	FAttachmentBeginCollision OnAttachmentBeginCollision;
	FAttachmentEndCollision OnAttachmentEndCollision;

	FAttachmentBeginOverlap OnAttachmentBeginOverlap;
	FAttachmentEndOverlap OnAttachmentEndOverlap;

	TWeakObjectPtr<ACCommonCharacter> Owner;
};