#include "Weapons/CMagicYonduSkillContext.h"

#include "Utilities/CLog.h"
#include "Characters/CCommonCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Weapons/CSkillYondu.h"

void ACMagicYonduSkillContext::DoAction_Implementation(CEAttackType const InAttackType, int32 const InSkillIndex)
{
	Super::DoAction_Implementation(InAttackType, InSkillIndex);

	(void)InAttackType;
	(void)InSkillIndex;

	const int32 RowCount = DoActionDatas.Num();
	const bool HasRowsScratch = RowCount > 0;
	const bool IdleOrRidingScratch = true == Magic_IsOwnerIdle() || true == Magic_IsOwnerRiding();

	if ((false == HasRowsScratch) || (false == IdleOrRidingScratch))
	{
		return;
	}

	Magic_SetOwnerStateAction();
	PlayAction(DoActionDatas, 0);
}

void ACMagicYonduSkillContext::Begin_DoAction_Implementation(CEAttackType const InAttackType)
{
	Super::Begin_DoAction_Implementation(InAttackType);

	(void)InAttackType;

	ACharacter* AsCharacterScratch = Cast<ACharacter>(Character.Get());

	if (false == IsValid(AsCharacterScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicYondu] Character 없음 — 스폰 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	USkeletalMeshComponent* MeshScratch = AsCharacterScratch->GetMesh();

	if (false == IsValid(MeshScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicYondu] Mesh 없음 — 스폰 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	const FTransform SpawnTransformScratch = MeshScratch->GetSocketTransform(YonduSpawnSocketName);

	UWorld* WorldScratch = GetWorld();

	if (false == IsValid(WorldScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicYondu] World 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	if (nullptr == SkillYonduActorClass.Get())
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicYondu] SkillYonduActorClass 미설정 — 컨텍스트에 `ACSkillYondu` 기본 블프를 지정해야 함 — %s"),
			*GetNameSafe(this)));
		return;
	}

	FActorSpawnParameters ParamsScratch;

	ParamsScratch.Owner = this;

	ParamsScratch.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACSkillYondu* SpawnedYonduScratch =
		WorldScratch->SpawnActor<ACSkillYondu>(SkillYonduActorClass, SpawnTransformScratch, ParamsScratch);

	if (false == IsValid(SpawnedYonduScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicYondu] Spawn 실패 Class=%s — %s"),
			*GetNameSafe(SkillYonduActorClass.Get()),
			*GetNameSafe(this)));
		return;
	}

	SpawnedYonduScratch->Shoot();
}
