#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/CInterface_Interactable.h"
#include "Commons/CEnums.h"
#include "Kismet/KismetSystemLibrary.h"
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
class UCCamComponent;
class UCMovementComponent;
class UCStateComponent;
class UAnimMontage;
class UCWeaponComponent;
class AController;
class USoundBase;

UENUM()
enum class CERidingState : uint8
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
enum class CERidingPoint : uint8
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
	void SetRidingState(const CERidingState InState);

	UFUNCTION()
		void BeginOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor, 
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, 
			bool bFromSweep, 
			const FHitResult& SweepResult);

	UFUNCTION()
		void EndOverlap(
			UPrimitiveComponent* OverlappedComponent, 
			AActor* OtherActor, 
			UPrimitiveComponent* OtherComp, 
			int32 OtherBodyIndex);

public:
	void SetInteractor(
		TWeakObjectPtr<ACCommonCharacter> InCharacter, 
		const TWeakObjectPtr<ACCommonCharacter> InOtherCharacter);

	UFUNCTION()
		void SetRider(ACCommonCharacter* InCharacter);

	FORCEINLINE TWeakObjectPtr<ACCommonCharacter> GetRider() const { return Rider; };

private:
	void Tick_ToMountPoint();
	void Tick_Mounting();
	void Tick_MountingEnd();
	void Tick_Riding() const;
	void Tick_Unmounting();
	void Tick_UnmountingEnd();

public:
	void CheckValidPoint();
	bool MoveToPoint(
		TWeakObjectPtr<ACCommonCharacter> Char, 
		const TWeakObjectPtr<USceneComponent> To);
	void PossessAndInterpToCamera();

	UFUNCTION()
		void InterpToRiderPos(UAnimMontage* Anim, bool bInterrupted);

	UFUNCTION()
		void AttachToRiderPoint(UAnimMontage* Anim, bool bInterrupted);

	void UnpossessAndInterpToCamera();

	UFUNCTION()
		void Unmount();

	void InputAction_Act();
	void Input_Zoom(const float InAxis) {}
	void Input_Targeting() {}

	void SetStatusUI() {}
	void OnStatusUI(const bool InOn) {}


private:
	UPROPERTY(EditDefaultsOnly, Category = "Interact")
		UTexture2D* InteractionKeyTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
		FText InteractionText;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		UAnimMontage* MountAnims[static_cast<uint8>(CEDirection::Max)];

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		USoundBase* MountSound;

	UPROPERTY(VisibleAnywhere, Category = "Mount")
		AController* ControllerSave;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		UAnimMontage* UnmountAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		USoundBase* UnmountSound;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		float OverTime_Camera = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		float OverTime_Mount = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		float OverTime_RiderPos = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		float OverTime_Unmount = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "IK")
		float LegIKAlpha = 0.2f;

private:
	TWeakObjectPtr<ACAnimal_AI> Owner;

	TWeakObjectPtr<USkeletalMeshComponent> Mesh;
	TWeakObjectPtr<USpringArmComponent> SpringArm;
	TWeakObjectPtr<UCameraComponent> Camera;
	TWeakObjectPtr<UCStateComponent> StateComp;
	TWeakObjectPtr<UCMovementComponent> MovementComp;
	TWeakObjectPtr<UCCamComponent> CamComp;
	TWeakObjectPtr<UCUserWidget_HUD> Hud;
	CERidingState RidingState;
	TWeakObjectPtr<ACCommonCharacter> Rider;
	TWeakObjectPtr<UCStateComponent> RiderStateComp;
	TWeakObjectPtr<UCMovementComponent> RiderMovementComp;
	TWeakObjectPtr<UCCamComponent> RiderCamComp;
	TWeakObjectPtr<UCWeaponComponent> RiderWeaponComp;
	TWeakObjectPtr<USceneComponent> RidingPoints[static_cast<uint8>(CERidingPoint::Max)];
	TWeakObjectPtr<UBoxComponent> InteractionCollision;
	TWeakObjectPtr<UCUserWidget_Interaction> Interaction;
	CEDirection MountDirection;
	float MountRotationZFactor = 0.0f;
	float Zooming = 0.0f;

	TEnumAsByte<EMoveComponentAction::Type> eMoveAction;
	FLatentActionInfo latentInfo;
};