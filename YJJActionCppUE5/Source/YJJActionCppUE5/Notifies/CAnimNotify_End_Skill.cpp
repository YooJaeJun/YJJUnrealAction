#include "Notifies/CAnimNotify_End_Skill.h"

#include "Global.h"
#include "Components/CWeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapons/CSkill.h"
#include "GameFramework/Actor.h"

FString UCAnimNotify_End_Skill::GetNotifyName_Implementation() const
{
	return TEXT("End_Skill");
}

void UCAnimNotify_End_Skill::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (false == IsValid(MeshComp))
		return;

	AActor* const ownerActor = MeshComp->GetOwner();
	if (false == IsValid(ownerActor))
		return;

	UCWeaponComponent* const weaponComp = YJJHelpers::GetComponent<UCWeaponComponent>(ownerActor);
	if (false == IsValid(weaponComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[AN_End_Skill] UCWeaponComponent 없음 — Owner=%s"),
			*ownerActor->GetName()));
		return;
	}

	weaponComp->LegacyBp_DispatchMain_EndSkill();

	UCSkill* const skill = weaponComp->GetSkill(SkillIndex);
	if (IsValid(skill))
		skill->End_Skill();
}
