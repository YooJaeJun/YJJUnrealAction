#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Component/CWeaponComponent.h"
#include "CAnimInstance_Character.generated.h"

class ACPlayer;
class UCWeaponComponent;

UCLASS()
class YJJACTIONCPP_API UCAnimInstance_Character : public UAnimInstance
{
	GENERATED_BODY()
		
protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
		float Speed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
		float Pitch;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
		float Direction;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapons")
		EWeaponType WeaponType = EWeaponType::Max;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapons")
		EWeaponType WeaponPrevType = EWeaponType::Max;

public:
	void NativeBeginPlay() override;
	void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UFUNCTION()
		void OnWeaponTypeChanged(EWeaponType InPrevType, EWeaponType InNewType);

private:
	TWeakObjectPtr<ACPlayer> Owner;
	TWeakObjectPtr<UCWeaponComponent> Weapon;

private:
	FRotator PrevRotation;
};
