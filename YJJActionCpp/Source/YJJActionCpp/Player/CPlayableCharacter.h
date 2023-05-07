#pragma once
#include "CoreMinimal.h"
#include "Character/CCommonCharacter.h"
#include "Character/CInterface_CharacterAnim.h"
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
	public ACCommonCharacter,
	public ICInterface_CharacterAnim
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
	void End_Avoid() override;
	void InputAction_Avoid();

private:
	UFUNCTION()
		void OnStateTypeChanged(const EStateType InPrevType, const EStateType InNewType);

public:
	// ZoomComponent에서 사용
	UPROPERTY(VisibleAnywhere)
		UCTargetingComponent* TargetingComponent;

	UPROPERTY(VisibleAnywhere)
		USpringArmComponent* SpringArm;

private:
	UPROPERTY(VisibleAnywhere)
		UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
		UCWeaponComponent* WeaponComponent;

	UPROPERTY(VisibleAnywhere)
		UCMontagesComponent* MontagesComponent;

	UPROPERTY(VisibleAnywhere)
		UCZoomComponent* ZoomComponent;

	UPROPERTY(VisibleAnywhere)
		UCGameUIComponent* GameUIComponent;

	UPROPERTY(EditAnywhere, Category = "UI")
		UWidgetComponent* InfoWidgetComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		FVector2D PitchRange = FVector2D(-40, +40);
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		TArray<float> Speeds{ 200, 500, 800 };
};
