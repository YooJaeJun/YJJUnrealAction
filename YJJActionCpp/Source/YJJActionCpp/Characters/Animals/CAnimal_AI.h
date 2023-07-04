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

	FORCEINLINE constexpr UBoxComponent* GetInteractionCollision() const { return InteractionCollision; }
	FORCEINLINE constexpr USceneComponent* GetMountLeftPoint() const { return MountLeftPoint; }
	FORCEINLINE constexpr USceneComponent* GetMountRightPoint() const { return MountRightPoint; }
	FORCEINLINE constexpr USceneComponent* GetMountBackPoint() const { return MountBackPoint; }
	FORCEINLINE constexpr USceneComponent* GetRiderPoint() const { return RiderPoint; }
	FORCEINLINE constexpr USceneComponent* GetUnmountPoint() const { return UnmountPoint; }
	FORCEINLINE constexpr USceneComponent* GetEyePoint() const { return EyePoint; }

	FORCEINLINE constexpr USpringArmComponent* GetSpringArm() const { return SpringArm; }
	FORCEINLINE constexpr UCameraComponent* GetCamera() const { return Camera; }
	FORCEINLINE constexpr UCTargetingComponent* GetTargetingComp() const { return TargetingComp; }
	FORCEINLINE constexpr UCZoomComponent* GetZoomComp() const { return ZoomComp; }

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
