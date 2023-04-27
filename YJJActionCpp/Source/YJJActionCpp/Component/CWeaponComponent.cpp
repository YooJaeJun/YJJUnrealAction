#include "Component/CWeaponComponent.h"
#include "Global.h"
#include "Character/CCharacter.h"
#include "Weapons/CWeapon.h"

UCWeaponComponent::UCWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ACCharacter>(GetOwner());
	CheckNull(Owner);

	FActorSpawnParameters params;
	params.Owner = Owner.Get();
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}

void UCWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCWeaponComponent::SetUnarmedMode()
{
}

void UCWeaponComponent::SetSwordMode()
{
}

void UCWeaponComponent::SetFistMode()
{
}

void UCWeaponComponent::SetHammerMode()
{
}

void UCWeaponComponent::SetBowMode()
{
}

void UCWeaponComponent::SetDualMode()
{
}

void UCWeaponComponent::Begin_Equip()
{
}

void UCWeaponComponent::End_Equip()
{
}

void UCWeaponComponent::Begin_Act()
{
}

void UCWeaponComponent::End_Act()
{
}
