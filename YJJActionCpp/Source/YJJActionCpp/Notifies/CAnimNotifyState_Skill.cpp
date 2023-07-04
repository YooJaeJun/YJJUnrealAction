#include "Notifies/CAnimNotifyState_Skill.h"
#include "Global.h"
#include "Components/CWeaponComponent.h"
#include "Weapons/CSkill.h"

FString UCAnimNotifyState_Skill::GetNotifyName_Implementation() const
{
	return "Skill";
}

void UCAnimNotifyState_Skill::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	const TWeakObjectPtr<UCWeaponComponent> weapon = 
		YJJHelpers::GetComponent<UCWeaponComponent>(MeshComp->GetOwner());
	CheckNull(weapon);
	CheckNull(weapon->GetSkill());

	weapon->GetSkill()->Begin_Skill();
}

void UCAnimNotifyState_Skill::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	const TWeakObjectPtr<UCWeaponComponent> weapon = 
		YJJHelpers::GetComponent<UCWeaponComponent>(MeshComp->GetOwner());
	CheckNull(weapon);
	CheckNull(weapon->GetSkill());

	weapon->GetSkill()->End_Skill();
}