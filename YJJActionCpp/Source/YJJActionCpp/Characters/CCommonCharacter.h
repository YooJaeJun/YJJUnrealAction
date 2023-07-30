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
class YJJACTIONCPP_API ACCommonCharacter :
	public ACharacter,
	public ICInterface_CharacterState,
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
	virtual void End_Dead() override;
	virtual void End_Rise() override;

	FORCEINLINE void SetbRiding(const bool InbRiding) { bRiding = InbRiding; }
	FORCEINLINE constexpr bool GetbRiding() const { return bRiding; }

	// Anim Instance 에서 캐스팅하지 않고 부르기 위함
	virtual USpringArmComponent* GetSpringArm() const
		PURE_VIRTUAL(ACCommonCharacter::GetSpringArm, return nullptr;);

	virtual UCTargetingComponent* GetTargetingComp() const
		PURE_VIRTUAL(ACCommonCharacter::GetTargetingComp, return nullptr;);

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
	void SetInteractor(ACCommonCharacter* InCharacter);
	FORCEINLINE constexpr ACCommonCharacter* GetInteractor() const { return Interactor; };

	FORCEINLINE constexpr UFXSystemAsset* GetLandEffect() const { return LandEffect; }
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
		UCStateComponent* StateComp;

	UPROPERTY(VisibleAnywhere)
		UCMovementComponent* MovementComp;

	UPROPERTY(VisibleAnywhere)
		UCMontagesComponent* MontagesComp;

	UPROPERTY(VisibleAnywhere, Category = "Status")
		UCCharacterInfoComponent* CharacterInfoComp;
	
	UPROPERTY(VisibleAnywhere, Category = "Status")
		UCCharacterStatComponent* CharacterStatComp;


	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
		USceneComponent* TargetingPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
		UWidgetComponent* TargetingWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
		TSubclassOf<UCUserWidget_Custom> TargetingWidget;


	UPROPERTY(EditDefaultsOnly, Category = "Info")
		USceneComponent* InfoPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
		UWidgetComponent* InfoWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
		TSubclassOf<UCUserWidget_EnemyBar> InfoWidget;


	UPROPERTY(EditDefaultsOnly, Category = "Land")
		USoundBase* LandSound;

	UPROPERTY(EditDefaultsOnly, Category = "Land")
		UFXSystemAsset* LandEffect;

	UPROPERTY(EditAnywhere, Category = "Land")
		float LandEffectScaleFactor = 1.0f;


	UPROPERTY(EditAnywhere, Category = "Color")
		FLinearColor OriginColor = FLinearColor::White;

private:
	UPROPERTY(VisibleAnywhere, Category = "Key")
		FVector2D MousePos;

	UPROPERTY(VisibleAnywhere)
		ACCommonCharacter* Interactor;

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
