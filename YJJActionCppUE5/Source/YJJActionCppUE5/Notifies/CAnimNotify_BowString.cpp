#include "Notifies/CAnimNotify_BowString.h"
#include "Global.h"
#include "Notifies/CAnimNotifyHelpers.h"
#include "Components/CWeaponComponent.h"
#include "Weapons/Acts/CAct_Bow.h"

FString UCAnimNotify_BowString::GetNotifyName_Implementation() const
{
	return TEXT("End_BowString");
}

void UCAnimNotify_BowString::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	UCWeaponComponent* weaponComp = CAnimNotifyHelpers_ResolveWeaponComponent(MeshComp);
	CheckNull(weaponComp);

	// 레거시 AN_End_BowString: Weapon_Bow AttachBowString = true
	weaponComp->ApplyLegacyEndBowStringAttach();

	// UCAct_Bow 경로 폴백 — MainWeapon 이 활 C++ 액터가 아닐 때
	const TObjectPtr<UCAct> act = weaponComp->GetAct();
	if (false == IsValid(act))
	{
		return;
	}

	const TWeakObjectPtr<UCAct_Bow> bowAct = Cast<UCAct_Bow>(act);
	if (false == bowAct.IsValid())
	{
		return;
	}

	bowAct->End_BowString();
}
