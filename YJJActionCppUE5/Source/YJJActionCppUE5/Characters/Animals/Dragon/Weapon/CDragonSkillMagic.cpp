#include "Characters/Animals/Dragon/Weapon/CDragonSkillMagic.h"

#include "Characters/Animals/Dragon/CDragon.h"
#include "Characters/Animals/Dragon/Weapon/CDragonWeapon.h"
#include "Characters/CCommonCharacter.h"

void ACSkillDragon::SkillDragon_AppendUniqueIgnoreActor(
	TArray<TObjectPtr<AActor>>& IgnoreListScratch,
	AActor* CandidateScratch)
{
	if (false == IsValid(CandidateScratch))
	{
		return;
	}

	const int32 countScratch = IgnoreListScratch.Num();
	for (int32 ixScratch = 0; ixScratch < countScratch; ++ixScratch)
	{
		if (IgnoreListScratch[ixScratch].Get() == CandidateScratch)
		{
			return;
		}
	}

	IgnoreListScratch.Add(TObjectPtr<AActor>(CandidateScratch));
}

void ACSkillDragon::SkillDragon_RebuildDragonRefsFromOwnerAndCharacter()
{
	AActor* const ownerScratch = GetOwner();

	ACDragonWeapon* const dragonWeaponOwnerScratch = Cast<ACDragonWeapon>(ownerScratch);
	ACDragon* const dragonOwnerScratch = Cast<ACDragon>(ownerScratch);

	DragonWeapon = dragonWeaponOwnerScratch;
	Dragon = dragonOwnerScratch;

	if ((false == IsValid(Dragon)) && IsValid(DragonWeapon))
	{
		Dragon = DragonWeapon->Dragon_ResolveOwningDragon();
	}

	if (false == IsValid(Dragon))
	{
		Dragon = Cast<ACDragon>(Character.Get());
	}

	if ((false == IsValid(DragonWeapon)) && IsValid(Dragon))
	{
		DragonWeapon = Cast<ACDragonWeapon>(Dragon->DragonWeapon.Get());
	}

	if ((false == IsValid(Dragon)) && (false == IsValid(DragonWeapon)))
	{
		UE_LOG(LogTemp,
			Warning,
			TEXT("[ACSkillDragon] Dragon/무기 레퍼런스를 확정하지 못함 — Owner(%s)·Character(%s) 확인."),
			*GetNameSafe(ownerScratch),
			*GetNameSafe(Character.Get()));
	}
}

ACSkillDragon::ACSkillDragon() = default;

void ACSkillDragon::BeginPlay()
{
	Super::BeginPlay();

	SkillDragon_RebuildDragonRefsFromOwnerAndCharacter();

	if (false == bAppendDefaultActorsToIgnoreOnBeginPlay)
	{
		return;
	}

	SkillDragon_AppendUniqueIgnoreActor(ActorsToIgnore, this);
	SkillDragon_AppendUniqueIgnoreActor(ActorsToIgnore, Dragon.Get());

	const ACCommonCharacter* const ccScratch = Character.Get();
	if (IsValid(ccScratch))
	{
		SkillDragon_AppendUniqueIgnoreActor(ActorsToIgnore, ccScratch->CurInteractingActor.Get());
	}
}
