#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/CCharacter.h"
#include "CPlayer.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class UCWeaponComponent;
class UInputComponent;
class UCAnimInstance_Character;

UCLASS()
class YJJACTIONCPP_API ACPlayer : public ACharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		FVector2D PitchRange = FVector2D(-40, +40);

private:
	UPROPERTY(VisibleAnywhere)
		USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
		UCameraComponent* Camera;

private:
	UPROPERTY(VisibleAnywhere)
		UCWeaponComponent* WeaponComponent;

public:
	ACPlayer();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void OnMoveForward(const float InAxisValue);
	void OnMoveRight(const float InAxisValue);
	void OnHorizontalLook(const float InAxisValue);
	void OnVerticalLook(const float InAxisValue);
	void OnJump();

private:
	void OnRun();
	void OffRun();
};
