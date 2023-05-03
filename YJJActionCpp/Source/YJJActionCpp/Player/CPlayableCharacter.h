#pragma once
#include "CoreMinimal.h"
#include "Character/CCommonCharacter.h"
#include "Character/CInterface_CharacterAnim.h"
#include "Character/CInterface_CharacterUI.h"
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

UCLASS()
class YJJACTIONCPP_API ACPlayableCharacter :
	public ACCommonCharacter,
	public ICInterface_CharacterAnim
{
	GENERATED_BODY()

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
	void OnAvoid();

private:
	UFUNCTION()
		void OnStateTypeChanged(const EStateType InPrevType, const EStateType InNewType);
};
