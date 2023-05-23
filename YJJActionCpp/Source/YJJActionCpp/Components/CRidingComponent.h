#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/CInterface_Interactable.h"
#include "CRidingComponent.generated.h"

class USkeletalMeshComponent;
class UBoxComponent;
class ACCommonCharacter;
class ACGameMode;
class UCUserWidget_HUD;
class UCUserWidget_Interaction;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YJJACTIONCPP_API UCRidingComponent :
	public UActorComponent,
	public ICInterface_Interactable
{
	GENERATED_BODY()

public:	
	UCRidingComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION()
		void BeginOverlap(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor, UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void EndOverlap(UPrimitiveComponent* OverlappedComponent, 
			AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	virtual void Interact(ACCommonCharacter* InteractingActor) override;

private:
	UPROPERTY()
		USkeletalMeshComponent* Mesh;

	UPROPERTY()
		UBoxComponent* InteractionCollision;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		UCUserWidget_HUD* Hud;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		UCUserWidget_Interaction* Interaction;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
};
