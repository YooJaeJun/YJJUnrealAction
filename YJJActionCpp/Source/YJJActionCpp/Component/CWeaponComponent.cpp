#include "Component/CWeaponComponent.h"
#include "Weapons/CWeapon.h"
#include "Player/CPlayer.h"
#include "Global.h"

UCWeaponComponent::UCWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ACPlayer>(GetOwner());
	CheckNull(Owner);

	FActorSpawnParameters params;
	params.Owner = Owner.Get();
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}

void UCWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCWeaponComponent::SetUnarmed()
{
}

void UCWeaponComponent::SetSword()
{
}

void UCWeaponComponent::SetFist()
{
}

void UCWeaponComponent::SetHammer()
{
}

void UCWeaponComponent::SetBow()
{
}

void UCWeaponComponent::SetDual()
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
