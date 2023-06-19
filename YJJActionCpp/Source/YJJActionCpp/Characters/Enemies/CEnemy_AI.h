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
class YJJACTIONCPP_API ACEnemy_AI : public ACEnemy
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
		void OnStateTypeChanged(EStateType InPrevType, EStateType InNewType);

private:
	virtual void Hit() override;
	virtual void End_Hit() override;

private:
	UPROPERTY(VisibleAnywhere)
		UCWeaponComponent* WeaponComp;

private:
	EAttackType CurAttackType = EAttackType::None;
};
