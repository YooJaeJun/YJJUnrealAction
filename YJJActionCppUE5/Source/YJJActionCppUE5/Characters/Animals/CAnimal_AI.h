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
class UWidgetComponent;

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

	virtual void InputAction_Interact() override;

protected:
	virtual UWidgetComponent* GetAnimalHpBarWidgetComponent() const override;

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

	// RidingComponent BPVar Eye / EyeClass — C++ 탈것 액터에만 존재.
	FORCEINLINE TSubclassOf<AActor> GetEyeActorClass() const { return EyeClass; }
	FORCEINLINE TObjectPtr<AActor> GetSpawnedEyeActor() const { return Eye; }

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

	// BP_Animal Variable 이름 "RidingComponent" 와 동일 객체 — 직렬화·에디터 표시용.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "RidingComponent"))
	TObjectPtr<UCRidingComponent> RidingComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCWeaponComponent> WeaponComp;

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

	// BP_Animal — 캡슐 하위 Scene(이름 "Scene") + HpBarWidget.
	UPROPERTY(VisibleAnywhere, Category = "UI", meta = (DisplayName = "Scene"))
	TObjectPtr<USceneComponent> HpBarSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (DisplayName = "HpBarWidget"))
	TObjectPtr<UWidgetComponent> HpBarWidgetComp;
};