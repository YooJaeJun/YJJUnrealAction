#include "Notifies/CAnimNotify_WeaponComboNotify.h"
#include "Notifies/CAnimNotifyHelpers.h"
#include "Components/CWeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"

FString UCAnimNotify_Begin_AirCombo::GetNotifyName_Implementation() const
{
	return TEXT("Begin_DoAirCombo");
}

void UCAnimNotify_Begin_AirCombo::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCWeaponComponent* weaponComp = CAnimNotifyHelpers_ResolveWeaponComponent(MeshComp);
	if (IsValid(weaponComp))
	{
		weaponComp->Begin_DoAirCombo();
	}
}

FString UCAnimNotify_End_AirCombo::GetNotifyName_Implementation() const
{
	return TEXT("End_DoAirCombo");
}

void UCAnimNotify_End_AirCombo::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCWeaponComponent* weaponComp = CAnimNotifyHelpers_ResolveWeaponComponent(MeshComp);
	if (IsValid(weaponComp))
	{
		weaponComp->End_DoAirCombo();
	}
}

FString UCAnimNotify_Begin_FlyingAttack::GetNotifyName_Implementation() const
{
	return TEXT("Begin_DoFlyingAttack");
}

void UCAnimNotify_Begin_FlyingAttack::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCWeaponComponent* weaponComp = CAnimNotifyHelpers_ResolveWeaponComponent(MeshComp);
	if (IsValid(weaponComp))
	{
		weaponComp->Begin_DoFlyingAttack();
	}
}

FString UCAnimNotify_End_FlyingAttack::GetNotifyName_Implementation() const
{
	return TEXT("End_DoFlyingAttack");
}

void UCAnimNotify_End_FlyingAttack::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCWeaponComponent* weaponComp = CAnimNotifyHelpers_ResolveWeaponComponent(MeshComp);
	if (IsValid(weaponComp))
	{
		weaponComp->End_DoFlyingAttack();
	}
}

FString UCAnimNotify_End_DownAttack::GetNotifyName_Implementation() const
{
	return TEXT("End_DoDownAttack");
}

void UCAnimNotify_End_DownAttack::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCWeaponComponent* weaponComp = CAnimNotifyHelpers_ResolveWeaponComponent(MeshComp);
	if (IsValid(weaponComp))
	{
		weaponComp->End_DoDownAttack();
	}
}
