#include "Notifies/CAnimNotifyState_Skill.h"
#include "Global.h"
#include "Components/CWeaponComponent.h"
#include "Weapons/CSkill.h"

FString UCAnimNotifyState_Skill::GetNotifyName_Implementation() const
{
	return "Skill";
}

void UCAnimNotifyState_Skill::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	const TWeakObjectPtr<UCWeaponComponent> weapon = 
		YJJHelpers::GetComponent<UCWeaponComponent>(MeshComp->GetOwner());
	CheckNull(weapon);
	CheckNull(weapon->GetSkill(SkillIndex));

	weapon->GetSkill(SkillIndex)->Begin_Skill();
}

void UCAnimNotifyState_Skill::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	const TWeakObjectPtr<UCWeaponComponent> weapon = 
		YJJHelpers::GetComponent<UCWeaponComponent>(MeshComp->GetOwner());
	CheckNull(weapon);
	CheckNull(weapon->GetSkill(SkillIndex));

	weapon->GetSkill(SkillIndex)->End_Skill();
}