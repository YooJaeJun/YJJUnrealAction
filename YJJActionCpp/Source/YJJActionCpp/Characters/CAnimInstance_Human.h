#pragma once

#include "CoreMinimal.h"
#include "Characters/CAnimInstance_Character.h"
#include "Components/CWeaponComponent.h"
#include "CAnimInstance_Human.generated.h"

class ACCommonCharacter;

UCLASS()
class YJJACTIONCPP_API UCAnimInstance_Human : public UCAnimInstance_Character
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UFUNCTION()
		void OnWeaponTypeChanged(const EWeaponType InPrevType, const EWeaponType InNewType);

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapons")
		EWeaponType WeaponType = EWeaponType::Unarmed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapons")
		EWeaponType WeaponPrevType = EWeaponType::Unarmed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		bool bRiding;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		bool bRidingFalling;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapons")
		bool BowAiming;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "IK")
		float LegIKAlpha;

private:
	TWeakObjectPtr<UCWeaponComponent> WeaponComp;
};
