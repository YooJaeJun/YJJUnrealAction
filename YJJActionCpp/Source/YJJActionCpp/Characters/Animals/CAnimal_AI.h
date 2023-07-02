#pragma once

#include "CoreMinimal.h"
#include "Characters/Animals/CAnimal.h"
#include "Components/CZoomComponent.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Components/CStateComponent.h"
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
class UCWeaponComponent;

UCLASS(Abstract)
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

protected:
	virtual void Hit() override;

private:
	UFUNCTION()
		void OnStateTypeChanged(const EStateType InPrevType, const EStateType InNewType);

public:
	void SetZoomMinRange(const float InMinRange) const;
	void SetZoomMaxRange(const float InMaxRange) const;

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

	UPROPERTY(EditAnywhere)
		UCWeaponComponent* WeaponComp;

private:
	UPROPERTY(VisibleAnywhere)
		USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
		UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		TArray<float> Speeds{ 400, 1000, 1600 };

	UPROPERTY(VisibleAnywhere, Category = "AI")
		UCPatrolComponent* PatrolComp;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		USceneComponent* MountLeftPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		USceneComponent* MountRightPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		USceneComponent* MountBackPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		USceneComponent* RiderPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		USceneComponent* UnmountPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		USceneComponent* EyePoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		TSubclassOf<AActor> EyeClass;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		AActor* Eye;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		UBoxComponent* InteractionCollision;
};
