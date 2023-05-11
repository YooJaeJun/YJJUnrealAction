#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/CInterface_CharacterAnim.h"
#include "Character/CInterface_CharacterBody.h"
#include "CCommonCharacter.generated.h"

class UCStateComponent;
class UCMovementComponent;
class UCMontagesComponent;
class UCCharacterInfoComponent;
class UCCharacterStatComponent;
struct FActDamageEvent;
class UCWeaponStructures;
class UCInterface_CharacterBody;

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
	// 라이딩 시 PlayerController와 AIController 전환 시 사용. 다른 클래스에서 접근 시 GetController 등이 아닌, 이 함수로 접근
	FORCEINLINE TWeakObjectPtr<AController> GetMyCurController() const { return MyCurController; }

public:
	UFUNCTION()
		void SetTickLerp(FRotator InRotator);

	void TogglebTickLerpForTarget();

public:
	FORCEINLINE const FVector2D& GetMousePos() { return MousePos; }
	void SetMousePos(const FVector2D InPos);

private:
	UPROPERTY()
		FVector2D MousePos;

public:
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

public:
	TWeakObjectPtr<AController> MyCurController;

protected:
	FTimerHandle RestoreColor_TimerHandle;

protected:
	struct FDamageData
	{
		float Power;
		ACCommonCharacter* Character;
		AActor* Causer;

		const FActDamageEvent* Event;
	} Damage;

private:
	bool bTickLerpForTarget = false;
	FRotator TargetRotator = FRotator(0, 0, 0);
};
