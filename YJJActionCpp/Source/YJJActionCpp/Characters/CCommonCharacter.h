#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Characters/CInterface_CharacterAnim.h"
#include "Characters/CInterface_CharacterBody.h"
#include "Characters/CInterface_IK.h"
#include "CCommonCharacter.generated.h"

class UCStateComponent;
class UCMovementComponent;
class UCMontagesComponent;
class UCCharacterInfoComponent;
class UCCharacterStatComponent;
struct FActDamageEvent;
class UCWeaponStructures;
class ACAnimal_AI;
class USoundBase;
class UFXSystemAsset;
class UWidgetComponent;
class UCUserWidget_Custom;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMount, ACCommonCharacter*, Object);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUnmount);

UCLASS(Abstract)
class YJJACTIONCPP_API ACCommonCharacter :
	public ACharacter,
	public ICInterface_CharacterAnim,
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
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void Rise();
	virtual void Hit();
	virtual void Dead();

public:
	virtual void End_Hit() override;
	virtual void End_Dead() override;
	virtual void End_Rise() override;

	FORCEINLINE void SetbRiding(const bool InbRiding) { bRiding = InbRiding; }
	FORCEINLINE const bool GetbRiding() const { return bRiding; }

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
	void SetInteractor(ACCommonCharacter* InCharacter);
	FORCEINLINE ACCommonCharacter* GetInteractor() const { return Interactor; };

public:
	void InputAction_Interact();

public:
	UPROPERTY()
		FMount OnMount;

	UPROPERTY()
		FUnmount OnUnmount;

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

	UPROPERTY(EditDefaultsOnly, Category = "Land")
		USoundBase* LandSound;

	UPROPERTY(EditDefaultsOnly, Category = "Land")
		UFXSystemAsset* LandEffect;

	UPROPERTY(EditAnywhere, Category = "Land")
		float LandEffectScaleFactor = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
		USceneComponent* TargetingPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
		UWidgetComponent* TargetingWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
		TSubclassOf<UCUserWidget_Custom> TargetingWidget;

protected:
	UPROPERTY(EditAnywhere, Category = "Color")
		FLinearColor OriginColor = FLinearColor::White;

private:
	UPROPERTY(VisibleAnywhere, Category = "Key")
		FVector2D MousePos;

	UPROPERTY(VisibleAnywhere)
		ACCommonCharacter* Interactor;

	UPROPERTY(VisibleAnywhere, Category = "Ride")
		bool bRiding;

public:
	TWeakObjectPtr<AController> MyCurController;

protected:
	FTimerHandle RestoreColor_TimerHandle;

protected:
	struct FDamageData
	{
		float Power;
		const ACCommonCharacter* Attacker;
		const AActor* Causer;

		const FActDamageEvent* Event;
	} Damage;

private:
	bool bTickLerpForTarget = false;
	FRotator TargetRotator = FRotator(0, 0, 0);
};
