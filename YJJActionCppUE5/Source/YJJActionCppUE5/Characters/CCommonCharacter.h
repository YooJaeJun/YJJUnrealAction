#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/CInterface_CharacterState.h"
#include "Interfaces/CInterface_CharacterBody.h"
#include "Interfaces/CInterface_IK.h"
#include "Weapons/CWeaponStructures.h"
#include "CCommonCharacter.generated.h"

class UCStateComponent;
class UCMovementComponent;
class UCMontagesComponent;
class UCCharacterInfoComponent;
class UCCharacterStatComponent;
class UCWeaponStructures;
class ACAnimal_AI;
class USoundBase;
class UFXSystemAsset;
class UWidgetComponent;
class UCUserWidget_Custom;
class USceneComponent;
class UCUserWidget_EnemyBar;
class USpringArmComponent;
class UCTargetingComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMount, ACCommonCharacter*, Object);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUnmount);

USTRUCT()
struct FDamageData
{
	GENERATED_BODY()

	float Power;
	TWeakObjectPtr<ACCommonCharacter> Attacker;
	TWeakObjectPtr<AActor> Causer;
	FActDamageEvent Event;
};

UCLASS(Abstract)
class YJJACTIONCPPUE5_API ACCommonCharacter :
	public ACharacter,
	public ICInterface_CharacterState,
	public ICInterface_CharacterBody,
	public ICInterface_IK
{
	GENERATED_BODY()

public:
	ACCommonCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult& Hit) override;

public:
	virtual float TakeDamage(
		float DamageAmount, 
		FDamageEvent const& DamageEvent,
		AController* EventInstigator, 
		AActor* DamageCauser) override;

protected:
	virtual void Rise() override;
	virtual void Land() override;
	virtual void Hit() override;
	virtual void Dead() override;

public:
	virtual void End_Hit() override;
	virtual void End_Dead() override;
	virtual void End_Rise() override;

	FORCEINLINE void SetbRiding(const bool InbRiding) { bRiding = InbRiding; }
	FORCEINLINE constexpr bool GetbRiding() const { return bRiding; }

	virtual TObjectPtr<USpringArmComponent> GetSpringArm() const
		PURE_VIRTUAL(ACCommonCharacter::GetSpringArm, return nullptr;);

	virtual TObjectPtr<UCTargetingComponent> GetTargetingComp() const
		PURE_VIRTUAL(ACCommonCharacter::GetTargetingComp, return nullptr;);

private:
	UFUNCTION()
		void RestoreColor();

public:
	// 라이딩 시 PlayerController와 AIController 전환 시 사용
	void SetMyCurController(const TWeakObjectPtr<AController> InController);
	FORCEINLINE TWeakObjectPtr<AController> GetMyCurController() const { return MyCurController; }

public:
	UFUNCTION()
		void SetTickLerp(FRotator InRotator);

	void TogglebTickLerpForTarget();

public:
	void SetMousePos(const FVector2D InPos);
	FORCEINLINE const FVector2D& GetMousePos() const { return MousePos; }

public:
	void SetInteractor(TObjectPtr<ACCommonCharacter> InCharacter);
	FORCEINLINE TObjectPtr<ACCommonCharacter> GetInteractor() const { return Interactor; };

	FORCEINLINE TObjectPtr<UFXSystemAsset> GetLandEffect() const { return LandEffect; }
	FORCEINLINE constexpr float GetLandEffectScaleFactor() const { return LandEffectScaleFactor; }

public:
	void InputAction_Interact();

public:
	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		FMount OnMount;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
		FUnmount OnUnmount;

protected:
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UCStateComponent> StateComp;

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UCMovementComponent> MovementComp;

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UCMontagesComponent> MontagesComp;

	UPROPERTY(VisibleAnywhere, Category = "Status")
		TObjectPtr<UCCharacterInfoComponent> CharacterInfoComp;
	
	UPROPERTY(VisibleAnywhere, Category = "Status")
		TObjectPtr<UCCharacterStatComponent> CharacterStatComp;


	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
		TObjectPtr<USceneComponent> TargetingPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
		TObjectPtr<UWidgetComponent> TargetingWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
		TSubclassOf<UCUserWidget_Custom> TargetingWidget;


	UPROPERTY(EditDefaultsOnly, Category = "Info")
		TObjectPtr<USceneComponent> InfoPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
		TObjectPtr<UWidgetComponent> InfoWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
		TSubclassOf<UCUserWidget_EnemyBar> InfoWidget;


	UPROPERTY(EditDefaultsOnly, Category = "Land")
		TObjectPtr<USoundBase> LandSound;

	UPROPERTY(EditDefaultsOnly, Category = "Land")
		TObjectPtr<UFXSystemAsset> LandEffect;

	UPROPERTY(EditAnywhere, Category = "Land")
		float LandEffectScaleFactor = 1.0f;


	UPROPERTY(EditAnywhere, Category = "Color")
		FLinearColor OriginColor = FLinearColor::White;

private:
	UPROPERTY(VisibleAnywhere, Category = "Key")
		FVector2D MousePos;

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<ACCommonCharacter> Interactor;

	UPROPERTY(VisibleAnywhere, Category = "Ride")
		bool bRiding;

protected:
	FTimerHandle RestoreColor_TimerHandle;
	FDamageData Damage;

private:
	TWeakObjectPtr<AController> MyCurController;
	bool bTickLerpForTarget = false;
	FRotator TargetRotator = FRotator(0, 0, 0);
};
