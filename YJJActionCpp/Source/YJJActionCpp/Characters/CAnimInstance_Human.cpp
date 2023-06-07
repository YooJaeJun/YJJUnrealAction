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
			bRidingFalling = animal->StateComp->IsFallMode();
	}
}

void UCAnimInstance_Human::OnWeaponTypeChanged(const EWeaponType InPrevType, const EWeaponType InNewType)
{
	WeaponType = InNewType;
	WeaponPrevType = InPrevType;
}
