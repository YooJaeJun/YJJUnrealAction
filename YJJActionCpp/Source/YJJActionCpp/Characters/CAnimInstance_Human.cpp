#include "CAnimInstance_Human.h"
#include "Global.h"
#include "Animals/CAnimal_AI.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/CWeaponComponent.h"
#include "Player/CPlayableCharacter.h"

void UCAnimInstance_Human::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	WeaponComp = CHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	CheckNull(WeaponComp);
	WeaponComp->OnWeaponTypeChanged.AddUniqueDynamic(this, &UCAnimInstance_Human::OnWeaponTypeChanged);

	StateComp = CHelpers::GetComponent<UCStateComponent>(Owner.Get());
}

void UCAnimInstance_Human::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	CheckNull(Owner.Get());

	bRiding = Owner->GetbRiding();

	if (bRiding)
	{
		const auto animal = Cast<ACAnimal_AI>(Owner->GetMyCurController()->GetCharacter());

		if (!!animal)
		{
			bRidingFalling = StateComp->IsFallMode();

			Speed = animal->GetVelocity().Size();

			Look = CHelpers::GetLook(
				animal->GetCapsuleComponent()->GetForwardVector(),
				UKismetMathLibrary::GetForwardVector(Owner->GetControlRotation()),
				animal->GetCapsuleComponent()->GetUpVector());

			Look *= 180.0f;

			bFootIK = false;
			bRidingIK = true;

			LegIKAlpha = Owner->GetLegIKAlpha();
		}
	}//bRiding
	else
	{
		bFootIK = true;
		bRidingIK = false;
		Forward = 0.0f;
		Side = 0.0f;
	}
}

void UCAnimInstance_Human::OnWeaponTypeChanged(const EWeaponType InPrevType, const EWeaponType InNewType)
{
	WeaponType = InNewType;
	WeaponPrevType = InPrevType;
}
