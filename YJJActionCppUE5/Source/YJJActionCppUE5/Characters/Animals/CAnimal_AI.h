#pragma once

#include "CoreMinimal.h"
#include "Characters/Animals/CAnimal.h"
#include "Components/CCamComponent.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Components/CStateComponent.h"
#include "CAnimal_AI.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UCCamComponent;
class UCTargetingComponent;
class UCGameUIComponent;
class UCRidingComponent;
class UCPatrolComponent;
class UBoxComponent;
class CUserWidget_HUD;
class CUserWidget_Interaction;
class ACCommonCharacter;
class UCWeaponComponent;
class UCameraComponent;

UCLASS(Abstract)
class YJJACTIONCPPUE5_API ACAnimal_AI :
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
		void OnStateTypeChanged(const CEStateType InPrevType, const CEStateType InNewType);

	UFUNCTION()
		void OnHitStateTypeChanged(const CEHitType InPrevType, const CEHitType InNewType);

public:
	void SetZoomMinRange(const float InMinRange) const;
	void SetZoomMaxRange(const float InMaxRange) const;

	FORCEINLINE TObjectPtr<UBoxComponent> GetInteractionCollision() const { return InteractionCollision; }
	FORCEINLINE TObjectPtr<USceneComponent> GetMountLeftPoint() const { return MountLeftPoint; }
	FORCEINLINE TObjectPtr<USceneComponent> GetMountRightPoint() const { return MountRightPoint; }
	FORCEINLINE TObjectPtr<USceneComponent> GetMountBackPoint() const { return MountBackPoint; }
	FORCEINLINE TObjectPtr<USceneComponent> GetRiderPoint() const { return RiderPoint; }
	FORCEINLINE TObjectPtr<USceneComponent> GetUnmountPoint() const { return UnmountPoint; }
	FORCEINLINE TObjectPtr<USceneComponent> GetEyePoint() const { return EyePoint; }

	virtual TObjectPtr<USpringArmComponent> GetSpringArm() const override;
	virtual TObjectPtr<UCTargetingComponent> GetTargetingComp() const override;
	FORCEINLINE TObjectPtr<UCameraComponent> GetCamera() const { return Camera; }
	FORCEINLINE TObjectPtr<UCCamComponent> GetZoomComp() const { return CamComp; }

protected:
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UCCamComponent> CamComp;

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UCTargetingComponent> TargetingComp;

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UCGameUIComponent> GameUIComp;

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UCRidingComponent> RidingComp;

	UPROPERTY(EditAnywhere)
		TObjectPtr<UCWeaponComponent> WeaponComp;

private:
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		TArray<float> Speeds{ 400, 1000, 1600 };

	UPROPERTY(VisibleAnywhere, Category = "AI")
		TObjectPtr<UCPatrolComponent> PatrolComp;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		TObjectPtr<USceneComponent> MountLeftPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		TObjectPtr<USceneComponent> MountRightPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		TObjectPtr<USceneComponent> MountBackPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		TObjectPtr<USceneComponent> RiderPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		TObjectPtr<USceneComponent> UnmountPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		TObjectPtr<USceneComponent> EyePoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		TSubclassOf<AActor> EyeClass;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		TObjectPtr<AActor> Eye;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
		TObjectPtr<UBoxComponent> InteractionCollision;
};