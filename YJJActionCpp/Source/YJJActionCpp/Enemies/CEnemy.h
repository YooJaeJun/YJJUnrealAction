#pragma once

#include "CoreMinimal.h"
#include "Character/CCommonCharacter.h"
#include "Character/CInterface_CharacterBody.h"
#include "Weapons/CWeaponStructures.h"
#include "CEnemy.generated.h"

class AController;
class UCWeaponComponent;
struct FDamageEvent;

UCLASS()
class YJJACTIONCPP_API ACEnemy :
	public ACCommonCharacter
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
