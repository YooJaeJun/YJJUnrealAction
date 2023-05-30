#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/CInterface_Interactable.h"
#include "Game/CEnums.h"
#include "CRidingComponent.generated.h"

class USkeletalMeshComponent;
class UBoxComponent;
class ACCommonCharacter;
class ACGameMode;
class UCUserWidget_HUD;
class UCUserWidget_Interaction;
class ACAnimal_AI;
class AAIController;
class UTexture2D;

UENUM()
enum class ERidingState : uint8
{
	ToMountPoint,
	Mounting,
	MountingEnd,
	Riding,
	Unmounting,
	UnmountingEnd
};

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
	UPROPERTY(VisibleAnywhere, Category = "Settings")
		USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Settings")
		UBoxComponent* InteractionCollision;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		ERidingState RidingState;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		ACAnimal_AI* Owner;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		ACCommonCharacter* Rider;

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
		UCUserWidget_HUD* Hud;

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
		UCUserWidget_Interaction* Interaction;

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
		UTexture2D* InteractionKeyTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
		FText InteractionText;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		UAnimMontage* MountAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		float MountRotationZFactor = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		EDirection MountDir;

	//UPROPERTY(EditDefaultsOnly, Category = "Mount")
	//	AAIController* AIControllerSave;
};
