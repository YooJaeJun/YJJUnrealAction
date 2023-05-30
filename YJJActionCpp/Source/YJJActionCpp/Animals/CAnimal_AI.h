#pragma once

#include "CoreMinimal.h"
#include "Animals/CAnimal.h"
#include "Widgets/CUserWidget_HUD.h"
#include "CAnimal_AI.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UCZoomComponent;
class UCTargetingComponent;
class UCGameUIComponent;
class UCRidingComponent;
class UCPatrolComponent;
class UBoxComponent;
class CUserWidget_HUD;
class CUserWidget_Interaction;
class ACCommonCharacter;

UCLASS()
class YJJACTIONCPP_API ACAnimal_AI :
	public ACAnimal
{
	GENERATED_BODY()

public:
	ACAnimal_AI();

protected:
	virtual void BeginPlay() override;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE UBoxComponent* GetInteractionCollision() { return InteractionCollision; }
	FORCEINLINE USceneComponent* GetMountRightPoint() { return MountRightPoint; }
	FORCEINLINE USceneComponent* GetMountLeftPoint() { return MountLeftPoint; }
	FORCEINLINE USceneComponent* GetMountBackPoint() { return MountBackPoint; }
	FORCEINLINE USceneComponent* GetRiderPoint() { return RiderPoint; }
	FORCEINLINE USceneComponent* GetUnmountPoint() { return UnmountPoint; }
	FORCEINLINE USceneComponent* GetEyePoint() { return EyePoint; }

	FORCEINLINE USpringArmComponent* GetSpringArm() { return SpringArm; }
	FORCEINLINE UCameraComponent* GetCamera() { return Camera; }
	FORCEINLINE UCTargetingComponent* GetTargetingComp() { return TargetingComp; }


private:
	UPROPERTY(VisibleAnywhere)
		USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
		UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
		UCTargetingComponent* TargetingComp;

	UPROPERTY(VisibleAnywhere)
		UCZoomComponent* ZoomComp;

	UPROPERTY(VisibleAnywhere)
		UCGameUIComponent* GameUIComp;

	UPROPERTY(VisibleAnywhere)
		UCPatrolComponent* PatrolComp;

	// Rider
	UPROPERTY(VisibleAnywhere)
		UCRidingComponent* RidingComp;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* MountRightPoint;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* MountLeftPoint;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* MountBackPoint;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* RiderPoint;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* UnmountPoint;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* EyePoint;

	UPROPERTY(VisibleAnywhere)
		UBoxComponent* InteractionCollision;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		TArray<float> Speeds{ 400, 1000, 1600 };
};
