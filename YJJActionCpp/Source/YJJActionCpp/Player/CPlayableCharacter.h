#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/CCommonCharacter.h"
#include "CPlayableCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class UCZoomComponent;
class UCTargetingComponent;
class UCWeaponComponent;
class UCStateComponent;
class UInputComponent;
class UCAnimInstance_Character;

UCLASS()
class YJJACTIONCPP_API ACPlayableCharacter : public ACCommonCharacter
{
	GENERATED_BODY()

public:
	// ZoomComponent���� ���
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
		UCZoomComponent* ZoomComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		FVector2D PitchRange = FVector2D(-40, +40);
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		TArray<float> Speeds{ 200, 500, 800 };
	
	UPROPERTY(EditAnywhere, Category = "Status")
		float MaxExp;

	UPROPERTY(EditAnywhere, Category = "Status")
		float Exp;

	UPROPERTY(EditAnywhere, Category = "Status")
		int32 Level = 1;

	UPROPERTY(EditAnywhere, Category = "Status")
		float MaxHp;

	UPROPERTY(EditAnywhere, Category = "Status")
		float Hp;

	UPROPERTY(EditAnywhere, Category = "Status")
		float MaxStamina;

	UPROPERTY(EditAnywhere, Category = "Status")
		float Stamina;

	UPROPERTY(EditAnywhere, Category = "Status")
		float StaminaAccelRestore;

	UPROPERTY(EditAnywhere, Category = "Status")
		float StaminaDefaultRestore;

	UPROPERTY(EditAnywhere, Category = "Status")
		float MaxMana;

	UPROPERTY(EditAnywhere, Category = "Status")
		float Mana;

	UPROPERTY(EditAnywhere, Category = "Status")
		float ManaAccelRestore;

	UPROPERTY(EditAnywhere, Category = "Status")
		float ManaDefaultRestore;

	UPROPERTY(EditAnywhere, Category = "Status")
		bool EnoughStamina;

	UPROPERTY(EditAnywhere, Category = "Status")
		bool EnoughMana;

	UPROPERTY(EditAnywhere, Category = "Status")
		float Zooming;

public:
	ACPlayableCharacter();

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
	void OnZoom(const float InAxisValue);
	void OnWalk();
	void OnRun();
	void OnJump();
	void OnTargeting();
};
