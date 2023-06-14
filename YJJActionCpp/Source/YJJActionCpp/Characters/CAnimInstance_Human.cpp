#include "CAnimInstance_Human.h"
#include "Global.h"
#include "Animals/CAnimal_AI.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CWeaponComponent.h"

void UCAnimInstance_Human::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	WeaponComp = CHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	CheckNull(WeaponComp);
	WeaponComp->OnWeaponTypeChanged.AddDynamic(this, &UCAnimInstance_Human::OnWeaponTypeChanged);
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
			Speed = animal->GetVelocity().Size2D();
			bRidingFalling = animal->StateComp->IsFallMode();

			const FRotator rotator = animal->GetVelocity().ToOrientationRotator();
			const FRotator rotator2 = animal->GetControlRotation();
			const FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(rotator, rotator2);

			PrevRotation = UKismetMathLibrary::RInterpTo(PrevRotation, delta, DeltaSeconds, 25);

			Direction = PrevRotation.Yaw;

			Pitch = UKismetMathLibrary::FInterpTo(
				Pitch, animal->GetBaseAimRotation().Pitch, DeltaSeconds, 25);
		}
	}
}

void UCAnimInstance_Human::OnWeaponTypeChanged(const EWeaponType InPrevType, const EWeaponType InNewType)
{
	WeaponType = InNewType;
	WeaponPrevType = InPrevType;
}
