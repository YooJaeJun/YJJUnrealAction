#pragma once
#include "CoreMinimal.h"
#include "Character/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "CPlayableCharacter.generated.h"

class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UCAnimInstance_Character;
class UCMontagesComponent;
class UCZoomComponent;
class UCTargetingComponent;
class UCWeaponComponent;
class UCGameUIComponent;
class UWidgetComponent;
class ACGameMode;

UCLASS()
class YJJACTIONCPP_API ACPlayableCharacter :
	public ACCommonCharacter
{
	GENERATED_BODY()

public:
	ACPlayableCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult& Hit) override;

private:
	void Avoid();

	virtual void End_Avoid() override;
	virtual void End_Hit() override;
	virtual void End_Rise() override;

	void InputAction_Avoid();

private:
	UFUNCTION()
		void OnStateTypeChanged(const EStateType InPrevType, const EStateType InNewType);

public:
	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	FORCEINLINE UCTargetingComponent* GetTargetingComp() const { return TargetingComp; }

private:
	UPROPERTY(VisibleAnywhere)
		USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
		UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
		UCWeaponComponent* WeaponComp;

	UPROPERTY(VisibleAnywhere)
		UCTargetingComponent* TargetingComp;

	UPROPERTY(VisibleAnywhere)
		UCZoomComponent* ZoomComp;

	UPROPERTY(VisibleAnywhere)
		UCGameUIComponent* GameUIComp;

	UPROPERTY(EditAnywhere, Category = "Mode")
		TWeakObjectPtr<ACGameMode> GameMode;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		FVector2D PitchRange = FVector2D(-40, +40);
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		TArray<float> Speeds{ 200, 500, 800 };
};
