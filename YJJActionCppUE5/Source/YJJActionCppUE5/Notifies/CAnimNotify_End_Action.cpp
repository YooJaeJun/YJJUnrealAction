#include "Notifies/CAnimNotify_End_Action.h"
#include "Components/CMagicComponent.h"
#include "Components/CWeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"

FString UCAnimNotify_End_Action::GetNotifyName_Implementation() const
{
	return TEXT("End_Action");
}

void UCAnimNotify_End_Action::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (false == IsValid(MeshComp))
		return;

	AActor* meshOwnerActor = MeshComp->GetOwner();
	if (false == IsValid(meshOwnerActor))
		return;

	UCMagicComponent* magicComp = meshOwnerActor->FindComponentByClass<UCMagicComponent>();
	UCWeaponComponent* weaponComp = meshOwnerActor->FindComponentByClass<UCWeaponComponent>();

	// 레거시 EventGraph 는 Magic 무효면 곧바로 두 번째(Weapon) End_DoAction 경로만 시도했다.
	if (false == IsValid(magicComp))
	{
		if (IsValid(weaponComp))
			weaponComp->End_DoAction(AttackType);
		return;
	}

	// 마법 타입 슬롯이 Unarmed 이면 스킬 End 는 Weapon 쪽 레거시 End_DoAction 으로 보냈다.
	if (magicComp->IsUnarmed())
	{
		if (IsValid(weaponComp))
			weaponComp->End_DoAction(AttackType);
		return;
	}

	magicComp->End_DoAction(AttackType);
}
