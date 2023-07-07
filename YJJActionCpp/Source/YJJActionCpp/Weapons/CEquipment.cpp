#include "Weapons/CEquipment.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CMovementComponent.h"
#include "Components/CStateComponent.h"

void UCEquipment::BeginPlay(TWeakObjectPtr<ACCommonCharacter> InOwner, const FEquipmentData& InData)
{
	Owner = InOwner;
	Data = InData;

	MovementComp = YJJHelpers::GetComponent<UCMovementComponent>(InOwner.Get());
	StateComp = YJJHelpers::GetComponent<UCStateComponent>(InOwner.Get());
}

void UCEquipment::Equip_Implementation()
{
	StateComp->SetEquipMode();

	if (Data.bCanMove == false)
		MovementComp->Stop();

	if (Data.bUseControlRotation)
		MovementComp->EnableControlRotation();

	const TWeakObjectPtr<UCMovementComponent> movement = 
		Cast<UCMovementComponent>(Owner->GetComponentByClass(UCMovementComponent::StaticClass()));

	movement->SetRunSpeed();

	if (!!Data.Montage)
		Owner->PlayAnimMontage(Data.Montage, Data.PlayRate);
	else
	{
		Begin_Equip();
		End_Equip();
	}

	if (!!Data.Sound)
		Data.PlaySoundWave(Owner.Get());
}

void UCEquipment::Begin_Equip_Implementation()
{
	bBeginEquip = true;

	if (OnEquipmentBeginEquip.IsBound())
		OnEquipmentBeginEquip.Broadcast();
}

void UCEquipment::End_Equip_Implementation()
{
	bBeginEquip = false;
	bEquipped = true;

	StateComp->SetIdleMode();
	MovementComp->Move();
}

void UCEquipment::Unequip_Implementation()
{
	bEquipped = false;
	MovementComp->DisableControlRotation();

	const TWeakObjectPtr<UCMovementComponent> movement = 
		Cast<UCMovementComponent>(Owner->GetComponentByClass(UCMovementComponent::StaticClass()));

	movement->SetSprintSpeed();

	if (OnEquipmentUnequip.IsBound())
		OnEquipmentUnequip.Broadcast();
}