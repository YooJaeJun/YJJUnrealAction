#include "CAnimInstance_Human.h"
#include "Global.h"
#include "Animals/CAnimal_AI.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/CWeaponComponent.h"
#include "Weapons/CSkill.h"

void UCAnimInstance_Human::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	WeaponComp = YJJHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	CheckNull(WeaponComp);
	WeaponComp->OnWeaponTypeChanged.AddUniqueDynamic(this, &UCAnimInstance_Human::OnWeaponTypeChanged);

	StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());
}

void UCAnimInstance_Human::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	CheckNull(Owner.Get());

	bRiding = Owner->GetbRiding();

	if (bRiding)
	{
		const TWeakObjectPtr<ACAnimal_AI> animal = Cast<ACAnimal_AI>(Owner->GetMyCurController()->GetCharacter());

		if (animal.IsValid())
		{
			bRidingFalling = StateComp->IsFallMode();

			Speed = animal->GetVelocity().Size();

			Look = YJJHelpers::GetLook(
				animal->GetCapsuleComponent()->GetForwardVector(),
				UKismetMathLibrary::GetForwardVector(Owner->GetControlRotation()),
				animal->GetCapsuleComponent()->GetUpVector());

			Look *= 180.0f;

			bFeetIK = false;
			bRidingIK = true;
			LegIKAlpha = Owner->GetLegIKAlpha();
		}
	}//bRiding
	else
	{
		bFeetIK = true;
		bRidingIK = false;
		Forward = 0.0f;
		Side = 0.0f;
	}

	CheckNull(WeaponComp);
	CheckNull(WeaponComp->GetSkill(0));

	bBowAiming = true;
	bBowAiming &= (WeaponType == CEWeaponType::Bow);
	bBowAiming &= (WeaponComp->GetSkill(0)->GetInAction());
}

void UCAnimInstance_Human::OnWeaponTypeChanged(const CEWeaponType InPrevType, const CEWeaponType InNewType)
{
	WeaponType = InNewType;
	WeaponPrevType = InPrevType;
}