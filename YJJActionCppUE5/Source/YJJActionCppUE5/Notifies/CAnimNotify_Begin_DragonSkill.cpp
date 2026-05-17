#include "Notifies/CAnimNotify_Begin_DragonSkill.h"

#include "Characters/Animals/Dragon/CDragon.h"
#include "Characters/Animals/Dragon/Weapon/CDragonWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Utilities/CLog.h"

FString UCAnimNotify_Begin_DragonSkill::GetNotifyName_Implementation() const
{
	return TEXT("Begin_DragonSkill");
}

void UCAnimNotify_Begin_DragonSkill::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	(void)Animation;
	(void)EventReference;

	if (false == IsValid(MeshComp))
	{
		return;
	}

	AActor* const meshOwnerScratch = MeshComp->GetOwner();
	if (false == IsValid(meshOwnerScratch))
	{
		CLog::Log(TEXT("[UCAnimNotify_Begin_DragonSkill] Mesh 소유 Actor 없음 — 노티 무시."));
		return;
	}

	if (false == meshOwnerScratch->HasAuthority())
	{
		return;
	}

	ACDragon* const dragonScratch = Cast<ACDragon>(meshOwnerScratch);
	if (false == IsValid(dragonScratch))
	{
		CLog::Log(FString::Printf(TEXT("[UCAnimNotify_Begin_DragonSkill] 소유 폰이 ACDragon 아님 — %s."),
			*GetNameSafe(meshOwnerScratch)));
		return;
	}

	AActor* const weaponActorScratch = dragonScratch->DragonWeapon.Get();
	ACDragonWeapon* const weaponScratch = Cast<ACDragonWeapon>(weaponActorScratch);
	if (false == IsValid(weaponScratch))
	{
		CLog::Log(FString::Printf(TEXT("[UCAnimNotify_Begin_DragonSkill] DragonWeapon 미해결 — Dragon=%s."),
			*GetNameSafe(dragonScratch)));
		return;
	}

	const uint8 skillByteScratch = static_cast<uint8>(SkillType);
	weaponScratch->Begin_Skill(skillByteScratch);
}
