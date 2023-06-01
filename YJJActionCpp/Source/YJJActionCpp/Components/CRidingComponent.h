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
class UTexture2D;
class USceneComponent;
class USpringArmComponent;
class UCameraComponent;
class UCMovementComponent;
class UCStateComponent;
class UAnimMontage;
class UCWeaponComponent;
class AController;

UENUM()
enum class ERidingState : uint8
{
	None,
	ToMountPoint,
	Mounting,
	MountingEnd,
	Riding,
	Unmounting,
	UnmountingEnd
};

UENUM()
enum class ERidingPoint : uint8
{
	CurMount,
	CandidateLeft,
	CandidateRight,
	CandidateBack,
	Rider,
	Unmount,
	Max
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

public:
	void SetInteractableCharacter(TWeakObjectPtr<ACCommonCharacter> InCharacter, 
		const TWeakObjectPtr<ACCommonCharacter> InOtherCharacter);

	UFUNCTION()
		void SetRider(ACCommonCharacter* InCharacter);

	FORCEINLINE const ACCommonCharacter* GetRider() const { return Rider; };

private:
	void Tick_ToMountPoint();
	void Tick_Mounting();
	void Tick_MountingEnd();
	void Tick_Riding();
	void Tick_Unmounting();
	void Tick_UnmountingEnd();

public:
	void CheckValidPoint();
	bool MoveToPoint(ACCommonCharacter* Char, const USceneComponent* To);

	UFUNCTION()
		void AttachToRiderPoint(UAnimMontage* Anim, bool bInterrupted);

	void Input_Zoom(const float InAxis) {}
	void Input_Targeting() {}
	void ApplyZoom() {}

	void CancelHitAnim() {}

	void SetStatusUI() {}
	void OnStatusUI() {}

private:
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		ACCommonCharacter* Rider;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		UCWeaponComponent* RiderWeaponComp;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		UCUserWidget_HUD* Hud;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		ERidingState RidingState;

	UPROPERTY(VisibleAnywhere, Category = "Settings")
		USkeletalMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		USpringArmComponent* SpringArm;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		UCMovementComponent* MovementComp;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		UCStateComponent* StateComp;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		USceneComponent* RidingPoints[static_cast<uint8>(ERidingPoint::Max)];

	UPROPERTY(VisibleAnywhere, Category = "Settings")
		UBoxComponent* InteractionCollision;

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
		UCUserWidget_Interaction* Interaction;

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
		UTexture2D* InteractionKeyTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
		FText InteractionText;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		EDirection MountDir;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		UAnimMontage* MountAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		float MountRotationZFactor = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		USoundBase* MountSound;

	UPROPERTY(EditDefaultsOnly, Category = "Zoom")
		float Zooming;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		AController* ControllerSave;

private:
	TWeakObjectPtr<ACAnimal_AI> Owner;
};
