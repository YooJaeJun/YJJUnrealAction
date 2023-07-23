#pragma once

#include "CoreMinimal.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Weapons/CWeaponStructures.h"
#include "CEnemy.generated.h"

class AController;
class UCWeaponComponent;
struct FDamageEvent;

UCLASS()
class YJJACTIONCPP_API ACEnemy :
	public ACCommonCharacter,
	public ICInterface_CharacterBody
{
	GENERATED_BODY()

public:
	ACEnemy();

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
	UFUNCTION()
		void RestoreColor();

private:
	virtual void Hit() override;
	virtual void End_Hit() override;
	virtual void End_Rise() override;

private:
	UPROPERTY(VisibleAnywhere)
		UCWeaponComponent* WeaponComp;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		TArray<float> Speeds{ 200, 400, 600 };

private:
	CEHitType CurHitType = CEHitType::Common;
};
