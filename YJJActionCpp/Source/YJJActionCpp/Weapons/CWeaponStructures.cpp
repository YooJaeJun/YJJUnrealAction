#include "Weapons/CWeaponStructures.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "Component/CStateComponent.h"
#include "Component/CMovementComponent.h"
#include "Animation/AnimMontage.h"

void FActData::Act(ACCommonCharacter* InOwner)
{
	TWeakObjectPtr<UCMovementComponent> movement = CHelpers::GetComponent<UCMovementComponent>(InOwner);

	if (!!movement.Get())
	{
		if (bFixedCamera)
			movement->EnableControlRotation();

		if (false == bCanMove)
			movement->Stop();
	}

	if (!!Montage)
		InOwner->PlayAnimMontage(Montage, PlayRate);
}