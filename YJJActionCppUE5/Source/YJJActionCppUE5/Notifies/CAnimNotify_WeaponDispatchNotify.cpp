#include "Notifies/CAnimNotify_WeaponDispatchNotify.h"
#include "Notifies/CAnimNotifyHelpers.h"
#include "Components/CWeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"

FString UCAnimNotify_End_DashAttack::GetNotifyName_Implementation() const
{
	return TEXT("End_DashAttack");
}

void UCAnimNotify_End_DashAttack::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCWeaponComponent* weaponComp = CAnimNotifyHelpers_ResolveWeaponComponent(MeshComp);
	if (IsValid(weaponComp))
	{
		weaponComp->End_DashAttack();
	}
}

FString UCAnimNotify_End_FallDownAttack::GetNotifyName_Implementation() const
{
	return TEXT("End_FallDownAttack");
}

void UCAnimNotify_End_FallDownAttack::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCWeaponComponent* weaponComp = CAnimNotifyHelpers_ResolveWeaponComponent(MeshComp);
	if (IsValid(weaponComp))
	{
		weaponComp->End_FallDown();
	}
}

FString UCAnimNotify_ConsumeStamina::GetNotifyName_Implementation() const
{
	return TEXT("Consume_Stamina");
}

void UCAnimNotify_ConsumeStamina::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCWeaponComponent* weaponComp = CAnimNotifyHelpers_ResolveWeaponComponent(MeshComp);
	if (false == IsValid(weaponComp))
	{
		return;
	}

	// 레거시 AN_ConsumeStamina: 부족 시 End_DoAction + 몽타주 중단은 컴포넌트에서 처리한다.
	(void)weaponComp->TryNotifyConsumeStamina(StaminaAmount);
}

FString UCAnimNotify_BoxCollision::GetNotifyName_Implementation() const
{
	return TEXT("BoxCollision");
}

void UCAnimNotify_BoxCollision::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCWeaponComponent* weaponComp = CAnimNotifyHelpers_ResolveWeaponComponent(MeshComp);
	if (IsValid(weaponComp))
	{
		weaponComp->ApplyLegacyMainWeaponBoxCollisions();
	}
}
