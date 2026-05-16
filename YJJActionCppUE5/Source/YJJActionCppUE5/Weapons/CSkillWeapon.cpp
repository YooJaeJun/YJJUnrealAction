#include "Weapons/CSkillWeapon.h"
#include "Characters/CCommonCharacter.h"
#include "Utilities/CLog.h"
#include "Weapons/CAttachment.h"

void ACMagicSkillContext::Equip_Implementation()
{
	// 장착 가시화·무기 상태는 블루프린트 Magic_C 가 오버라이드 한다.
}

void ACMagicSkillContext::Unequip_Implementation()
{
	// 벗기기 처리도 BP 가 담당하면 빈 구현으로 둔다.
}

void ACMagicSkillContext::Begin_Equip_Implementation()
{
}

void ACMagicSkillContext::End_Equip_Implementation()
{
}

void ACMagicSkillContext::Begin_Unequip_Implementation()
{
}

void ACMagicSkillContext::End_Unequip_Implementation()
{
}

void ACMagicSkillContext::DoAction_Implementation(CEAttackType InAttackType, int32 InSkillIndex)
{
}

void ACMagicSkillContext::Begin_DoAction_Implementation(CEAttackType InAttackType)
{
}

void ACMagicSkillContext::End_DoAction_Implementation(CEAttackType InAttackType)
{
}

ACSkillWeapon::ACSkillWeapon()
{
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
	if (ACMagicSkillContext* magicActor = Cast<ACMagicSkillContext>(ownerActor))
	{
		Magic = magicActor;
		Weapon = nullptr;
		Character = magicActor->Character;
		if (false == IsValid(Character))
			CLog::Log(FString::Printf(TEXT("[SkillWeapon] Magic 컨텍스트의 Character 미설정 — %s"), *GetName()));
		return;
	}

	// BP: Magic 실패 시 Owner → Weapon_C.
	if (ACWeaponSkillContext* weaponActor = Cast<ACWeaponSkillContext>(ownerActor))
	{
		Weapon = weaponActor;
		Magic = nullptr;
		Character = weaponActor->Character;
		if (false == IsValid(Character))
			CLog::Log(FString::Printf(TEXT("[SkillWeapon] Weapon 컨텍스트의 Character 미설정 — %s"), *GetName()));
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
