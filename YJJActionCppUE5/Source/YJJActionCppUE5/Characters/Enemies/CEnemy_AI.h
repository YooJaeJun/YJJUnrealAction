#pragma once

#include "CoreMinimal.h"
#include "Characters/Enemies/CEnemy.h"
#include "Components/CStateComponent.h"
#include "Weapons/CWeaponStructures.h"
#include "CEnemy_AI.generated.h"

class AController;
class UCWeaponComponent;
struct FDamageEvent;

UCLASS(Abstract)
class YJJACTIONCPPUE5_API ACEnemy_AI :
	public ACEnemy
{
	GENERATED_BODY()

public:
	ACEnemy_AI();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
		void OnStateTypeChanged(CEStateType InPrevType, CEStateType InNewType);

	UFUNCTION()
		void OnHitStateTypeChanged(const CEHitType InPrevType, const CEHitType InNewType);

private:
	virtual void Hit() override;
	virtual void End_Hit() override;
	virtual void End_Rise() override;

private:
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UCWeaponComponent> WeaponComp;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		TArray<float> Speeds{ 200, 400, 600 };

private:
	CEHitType CurHitType = CEHitType::Common;
};
