#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Components/CWeaponComponent.h"
#include "Components/CStateComponent.h"
#include "CAnimInstance_Human.generated.h"

class ACCommonCharacter;

UCLASS()
class YJJACTIONCPP_API UCAnimInstance_Human : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UFUNCTION()
		void OnWeaponTypeChanged(const EWeaponType InPrevType, const EWeaponType InNewType);

	UFUNCTION()
		void OnStateTypeChanged(const EStateType InPrevType, const EStateType InNewType);
		
protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
		float Speed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
		float Pitch;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
		float Direction;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		EStateType StateType = EStateType::Idle;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		EStateType StatePrevType = EStateType::Idle;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		bool Falling;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		bool Hitting;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapons")
		EWeaponType WeaponType = EWeaponType::Unarmed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapons")
		EWeaponType WeaponPrevType = EWeaponType::Unarmed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapons")
		bool BowAiming;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCWeaponComponent> WeaponComp;
	TWeakObjectPtr<UCStateComponent> StateComp;
	FRotator PrevRotation;
};
