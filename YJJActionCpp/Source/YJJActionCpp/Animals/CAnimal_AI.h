#pragma once

#include "CoreMinimal.h"
#include "Animals/CAnimal.h"
#include "Character/CInterface_Rideable.h"
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

UCLASS()
class YJJACTIONCPP_API ACAnimal_AI :
	public ACAnimal,
	public ICInterface_Rideable
{
	GENERATED_BODY()

public:
	ACAnimal_AI();

protected:
	virtual void BeginPlay() override;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ZoomComponent에서 사용
public:
	UPROPERTY(VisibleAnywhere)
		USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
		UCTargetingComponent* TargetingComp;

private:
	UPROPERTY(VisibleAnywhere)
		USceneComponent* RiderPoint;

	UPROPERTY(VisibleAnywhere)
		UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
		UCZoomComponent* ZoomComp;

	UPROPERTY(VisibleAnywhere)
		UCGameUIComponent* GameUIComp;

	UPROPERTY(VisibleAnywhere)
		UCPatrolComponent* PatrolComp;

	UPROPERTY(VisibleAnywhere)
		UCRidingComponent* RidingComp;

	UPROPERTY(VisibleAnywhere)
		UBoxComponent* InteractionCollision;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* MountRight;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* MountLeft;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* MountBack;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* Unmount;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* EyePoint;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		TArray<float> Speeds{ 400, 1000, 1600 };
};
