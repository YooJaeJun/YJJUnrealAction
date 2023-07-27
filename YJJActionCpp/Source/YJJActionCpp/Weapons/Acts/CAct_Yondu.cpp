#include "Weapons/Acts/CAct_Yondu.h"
#include "Global.h"
#include "Components/CStateComponent.h"
#include "Weapons/AddOns/SkillColliders/CSkillCollider_Yondu.h"

UCAct_Yondu::UCAct_Yondu()
{
}

void UCAct_Yondu::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);

	CheckNull(Yondu);

	const FVector direction = Owner->GetCapsuleComponent()->GetForwardVector();
	const FRotator rotation = direction.Rotation();
	const FVector location = Owner->GetMesh()->GetSocketLocation("Pelvis") + LocationFactor;

	DefaultTransform = FTransform(rotation, location, FVector::OneVector);

	Yondu->SetDefaultTransform(DefaultTransform);
}

void UCAct_Yondu::OnBeginEquip()
{
	Super::OnBeginEquip();

	CheckNull(Owner);

	FActorSpawnParameters params;
	params.Owner = Owner.Get();
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Yondu = Owner->GetWorld()->SpawnActor<ACSkillCollider_Yondu>(YonduClass, params);
}

void UCAct_Yondu::OnUnequip()
{
	Super::OnUnequip();

	Yondu->Destroy();
}

void UCAct_Yondu::Act()
{
	Super::Act();

	Yondu->Shoot();
}
