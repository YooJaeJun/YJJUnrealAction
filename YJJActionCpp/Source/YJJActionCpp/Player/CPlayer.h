#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/CCharacter.h"
#include "CPlayer.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class UCWeaponComponent;
class UCStateComponent;
class UInputComponent;
class UCAnimInstance_Character;

UCLASS()
class YJJACTIONCPP_API ACPlayer : public ACCharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		FVector2D PitchRange = FVector2D(-40, +40);

	UPROPERTY(VisibleAnywhere)
		USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
		UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
		UCWeaponComponent* WeaponComponent;

	UPROPERTY(EditAnywhere, Category = "Settings")
		float MaxExp;

	UPROPERTY(EditAnywhere, Category = "Settings")
		float Exp;

	UPROPERTY(EditAnywhere, Category = "Settings")
		int32 Level = 1;

	UPROPERTY(EditAnywhere, Category = "Settings")
		float MaxHp;

	UPROPERTY(EditAnywhere, Category = "Settings")
		float Hp;

	UPROPERTY(EditAnywhere, Category = "Settings")
		float MaxStamina;

	UPROPERTY(EditAnywhere, Category = "Settings")
		float Stamina;

	UPROPERTY(EditAnywhere, Category = "Settings")
		float StaminaAccelRestore;

	UPROPERTY(EditAnywhere, Category = "Settings")
		float StaminaDefaultRestore;

	UPROPERTY(EditAnywhere, Category = "Settings")
		float MaxMana;

	UPROPERTY(EditAnywhere, Category = "Settings")
		float Mana;

	UPROPERTY(EditAnywhere, Category = "Settings")
		float ManaAccelRestore;

	UPROPERTY(EditAnywhere, Category = "Settings")
		float ManaDefaultRestore;

	UPROPERTY(EditAnywhere, Category = "Settings")
		bool EnoughStamina;

	UPROPERTY(EditAnywhere, Category = "Settings")
		bool EnoughMana;

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
