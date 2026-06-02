#include "Weapons/CSkillWeaponActors.h"

#include "Characters/Animals/Dragon/CDragon.h"
#include "Characters/Animals/Dragon/Weapon/CDragonWeapon.h"
#include "Characters/CCommonCharacter.h"
#include "Components/SceneComponent.h"
#include "Utilities/CLog.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CMagic.h"
#include "Weapons/CWeapon.h"

ACSkillWeapon::ACSkillWeapon()
{
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	PrimaryActorTick.bCanEverTick = false;
}

void ACSkillWeapon::BeginPlay()
{
	Super::BeginPlay();

	const TObjectPtr<AActor> ownerActor = GetOwner();
	if (false == IsValid(ownerActor))
	{
		CLog::Log(FString::Printf(
			TEXT("[SkillWeapon] BeginPlay: Owner 없음 — ChildActorComponent 또는 Spawn 시 Owner 미설정 — %s"),
			*GetName()));
		return;
	}

	// BP: Owner → Magic 이면 Magic 변수·Magic.Character 할당.
	if (ACMagic* magicActor = Cast<ACMagic>(ownerActor))
	{
		Magic = magicActor;
		Weapon = nullptr;
		Character = magicActor->Character;
		if (false == IsValid(Character))
			CLog::Log(FString::Printf(TEXT("[SkillWeapon] Magic 컨텍스트의 Character 미설정 — %s"), *GetName()));
		return;
	}

	// BP: Magic 실패 시 Owner → Weapon_C.
	if (ACWeapon* weaponActor = Cast<ACWeapon>(ownerActor))
	{
		Weapon = weaponActor;
		Magic = nullptr;
		Character = weaponActor->Character;
		if (false == IsValid(Character))
			CLog::Log(FString::Printf(TEXT("[SkillWeapon] Weapon 컨텍스트의 Character 미설정 — %s"), *GetName()));
		return;
	}

	// 레거시 Skill_Magic 계열 Skill_Dragon 등 — 소유 주체가 드래곤 폰이거나 무기(ACDragonWeapon)인 경우(ACDragon 은 ACCommonCharacter 계열).
	if (ACDragon* dragonDirectScratch = Cast<ACDragon>(ownerActor))
	{
		Magic = nullptr;
		Weapon = nullptr;
		Character = dragonDirectScratch;
		return;
	}

	if (ACDragonWeapon* dragonWeaponScratch = Cast<ACDragonWeapon>(ownerActor))
	{
		Magic = nullptr;
		Weapon = nullptr;

		ACDragon* owningDragonScratch = dragonWeaponScratch->Dragon_ResolveOwningDragon();
		Character = owningDragonScratch;
		if (false == IsValid(Character))
		{
			CLog::Log(FString::Printf(TEXT("[SkillWeapon] ACDragonWeapon Owner 이지만 Dragon_ResolveOwningDragon 실패 — %s"),
				*GetNameSafe(this)));
		}
		return;
	}

	// 애착 액터만 C++ 로 둔 경우: ACAttachment::Owner 로 동일 정보를 채운다.
	if (ACAttachment* attachment = Cast<ACAttachment>(ownerActor))
	{
		Magic = nullptr;
		Weapon = nullptr;
		Character = attachment->Owner.Get();
		if (false == IsValid(Character))
			CLog::Log(
				FString::Printf(TEXT("[SkillWeapon] ACAttachment Owner 미설정 — %s"), *GetName()));
		return;
	}

	CLog::Log(FString::Printf(
		TEXT("[SkillWeapon] Owner 타입 불일치 — Magic/Weapon 마커 또는 ACAttachment 가 아님 — Skill=%s Owner=%s"),
		*GetName(),
		*ownerActor->GetName()));
}
