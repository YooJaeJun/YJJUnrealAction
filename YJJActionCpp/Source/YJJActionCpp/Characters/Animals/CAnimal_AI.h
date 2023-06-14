#pragma once

#include "CoreMinimal.h"
#include "Characters/Animals/CAnimal.h"
#include "Components/CZoomComponent.h"
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

	virtual void Landed(const FHitResult& Hit) override;

public:
	FORCEINLINE void SetZoomMaxRange(const float InMaxRange) { ZoomComp->ZoomData.MaxRange = InMaxRange; }

	FORCEINLINE UBoxComponent* GetInteractionCollision() const { return InteractionCollision; }
	FORCEINLINE USceneComponent* GetMountLeftPoint() const { return MountLeftPoint; }
	FORCEINLINE USceneComponent* GetMountRightPoint() const { return MountRightPoint; }
	FORCEINLINE USceneComponent* GetMountBackPoint() const { return MountBackPoint; }
	FORCEINLINE USceneComponent* GetRiderPoint() const { return RiderPoint; }
	FORCEINLINE USceneComponent* GetUnmountPoint() const { return UnmountPoint; }
	FORCEINLINE USceneComponent* GetEyePoint() const { return EyePoint; }

	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	FORCEINLINE UCTargetingComponent* GetTargetingComp() const { return TargetingComp; }
	FORCEINLINE UCZoomComponent* GetZoomComp() const { return ZoomComp; }

protected:
	UPROPERTY(VisibleAnywhere)
		UCZoomComponent* ZoomComp;

	UPROPERTY(VisibleAnywhere)
		UCTargetingComponent* TargetingComp;

	UPROPERTY(VisibleAnywhere)
		UCGameUIComponent* GameUIComp;

	UPROPERTY(VisibleAnywhere)
		UCRidingComponent* RidingComp;

private:
	UPROPERTY(VisibleAnywhere)
		USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
		UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
		UCPatrolComponent* PatrolComp;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* MountLeftPoint;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* MountRightPoint;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* MountBackPoint;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* RiderPoint;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* UnmountPoint;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* EyePoint;

	UPROPERTY(VisibleAnywhere)
		TSubclassOf<AActor> EyeClass;

	UPROPERTY(VisibleAnywhere)
		AActor* Eye;

	UPROPERTY(VisibleAnywhere)
		UBoxComponent* InteractionCollision;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		TArray<float> Speeds{ 400, 1000, 1600 };
};
