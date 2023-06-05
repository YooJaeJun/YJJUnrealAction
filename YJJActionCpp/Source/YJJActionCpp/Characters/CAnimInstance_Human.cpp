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

	if (StateComp.Get())
		bRiding = (StateComp->IsRideMode());

	if (bRiding)
	{
		const auto animal = Cast<ACAnimal_AI>(Owner->GetMyCurController()->GetCharacter());
		if (!!animal)
		{
			Speed = animal->GetVelocity().Size2D();

			bRidingFalling = animal->StateComp->IsFallMode();
		}
	}
	else if (!!Owner.Get())
		Speed = Owner->GetVelocity().Size2D();
}

void UCAnimInstance_Human::OnWeaponTypeChanged(const EWeaponType InPrevType, const EWeaponType InNewType)
{
	WeaponType = InNewType;
	WeaponPrevType = InPrevType;
}
