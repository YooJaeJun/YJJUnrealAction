#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Characters/CInterface_CharacterAnim.h"
#include "Characters/CInterface_CharacterBody.h"
#include "CCommonCharacter.generated.h"

class UCStateComponent;
class UCMovementComponent;
class UCMontagesComponent;
class UCCharacterInfoComponent;
class UCCharacterStatComponent;
struct FActDamageEvent;
class UCWeaponStructures;
class ACAnimal_AI;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteract, ACCommonCharacter*, Object);

UCLASS(Abstract)
class YJJACTIONCPP_API ACCommonCharacter :
	public ACharacter,
	public ICInterface_CharacterAnim,
	public ICInterface_CharacterBody
{
	GENERATED_BODY()

public:
	ACCommonCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

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

protected:
	void InputAction_Interact();

public:
	UPROPERTY()
		FInteract OnInteract;

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

protected:
	UPROPERTY(EditAnywhere, Category = "Color")
		FLinearColor OriginColor = FLinearColor::White;

private:
	UPROPERTY()
		FVector2D MousePos;

	UPROPERTY(VisibleAnywhere)
		ACCommonCharacter* Interactor;

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
