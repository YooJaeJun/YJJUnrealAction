#include "Weapons/CEquipment.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "Components/CMovementComponent.h"
#include "Components/CStateComponent.h"

void UCEquipment::BeginPlay(ACCommonCharacter* InOwner, const FEquipmentData& InData)
{
	Owner = InOwner;
	Data = InData;

	MovementComp = CHelpers::GetComponent<UCMovementComponent>(InOwner);
	StateComp = CHelpers::GetComponent<UCStateComponent>(InOwner);
}

void UCEquipment::Equip_Implementation()
{
	StateComp->SetEquipMode();

	if (Data.bCanMove == false)
		MovementComp->Stop();

	if (Data.bUseControlRotation)
		MovementComp->EnableControlRotation();

	if (!!Data.Montage)
		Owner->PlayAnimMontage(Data.Montage, Data.PlayRate);
	else
	{
		Begin_Equip();
		End_Equip();
	}
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

	MovementComp->Move();
	StateComp->SetIdleMode();
}

void UCEquipment::Unequip_Implementation()
{
	bEquipped = false;
	MovementComp->DisableControlRotation();

	if (OnEquipmentUnequip.IsBound())
		OnEquipmentUnequip.Broadcast();
}
