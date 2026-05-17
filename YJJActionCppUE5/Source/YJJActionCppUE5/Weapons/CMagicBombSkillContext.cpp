#include "Weapons/CMagicBombSkillContext.h"

#include "Utilities/CLog.h"
#include "Characters/CCommonCharacter.h"
#include "Weapons/CSkillBomb.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

const FName ACMagicBombSkillContext::BombAiBlackboardTargetKey(TEXT("Target"));
const FName ACMagicBombSkillContext::BombSpawnSocketName(TEXT("Hand_FireBall"));

void ACMagicBombSkillContext::DoAction_Implementation(CEAttackType const InAttackType, int32 const InSkillIndex)
{
	(void)InAttackType;
	(void)InSkillIndex;

	Super::DoAction_Implementation(InAttackType, InSkillIndex);

	const int32 RowCount = DoActionDatas.Num();
	const bool bHasRows = RowCount > 0;
	const bool bIdleOrRiding = true == Magic_IsOwnerIdle() || true == Magic_IsOwnerRiding();

	if ((false == bHasRows) || (false == bIdleOrRiding))
	{
		return;
	}

	Magic_SetOwnerStateAction();
	PlayAction(DoActionDatas, 0);
}

void ACMagicBombSkillContext::Begin_DoAction_Implementation(CEAttackType const InAttackType)
{
	Super::Begin_DoAction_Implementation(InAttackType);

	(void)InAttackType;

	// 순서는 레거시 ExecutionSequence 과 동일: 방향 확정 → 소켓 위치 → 스폰·Shoot
	if (IsValid(Controller.Get()))
	{
		// AController 는 엔진에서 Transform 앞벡터 API 가 막혀 있음 — 시야 회전 기준 방향만 사용한다.
		const FRotator controlRot = Controller->GetControlRotation();
		BombDirection = UKismetMathLibrary::GetForwardVector(controlRot);
	}
	else
	{
		FVector fromBb = FVector::ZeroVector;
		const bool bBbOk = Bomb_TryResolveDirectionFromBlackboard(fromBb);
		if (false == bBbOk)
		{
			BombDirection = FVector::ZeroVector;
			CLog::Log(FString::Printf(
				TEXT("[MagicBomb] Controller 없고 블랙보드 Target 방향 계산 실패 — Direction 생략(0 벡터) — %s"),
				*GetNameSafe(this)));
		}
		else
		{
			BombDirection = fromBb;
		}
	}

	ACharacter* asCharacterForMesh = Cast<ACharacter>(Character.Get());
	if (false == IsValid(asCharacterForMesh))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicBomb] Begin_DoAction: Character 없음 — 폭탄 소켓·스폰 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	USkeletalMeshComponent* mesh = asCharacterForMesh->GetMesh();
	if (false == IsValid(mesh))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicBomb] Begin_DoAction: Mesh 없음 — 폭탄 스폰 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	BombSpawnLocation = mesh->GetSocketLocation(BombSpawnSocketName);

	UWorld* World = GetWorld();
	if (false == IsValid(World))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicBomb] Begin_DoAction: World 무효 — 스폰 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	if (nullptr == SkillBombActorClass.Get())
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicBomb] SkillBombActorClass 미설정 — 스폰 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	const FRotator SpawnRotation = UKismetMathLibrary::Conv_VectorToRotator(BombDirection);
	const FTransform SpawnTransform(SpawnRotation, BombSpawnLocation, FVector::OneVector);

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACSkillBomb* spawnedBomb = World->SpawnActor<ACSkillBomb>(SkillBombActorClass, SpawnTransform, Params);
	if (false == IsValid(spawnedBomb))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicBomb] SpawnActor 실패 — Class=%s — %s"),
			*GetNameSafe(SkillBombActorClass.Get()),
			*GetNameSafe(this)));
		return;
	}

	spawnedBomb->Shoot(BombDirection);
}

bool ACMagicBombSkillContext::Bomb_TryResolveDirectionFromBlackboard(FVector& OutDirection) const
{
	ACCommonCharacter* const ownerChar = Character.Get();
	if (false == IsValid(ownerChar))
	{
		return false;
	}

	UBlackboardComponent* bb = UAIBlueprintHelperLibrary::GetBlackboard(ownerChar);
	if (false == IsValid(bb))
	{
		return false;
	}

	UObject* targetRaw = bb->GetValueAsObject(BombAiBlackboardTargetKey);
	AActor* targetActorResolved = Cast<AActor>(targetRaw);
	if (false == IsValid(targetActorResolved))
	{
		return false;
	}

	const FVector EnemyLocation = ownerChar->GetActorLocation();
	const FVector TargetLocation = targetActorResolved->GetActorLocation();
	FVector Diff = TargetLocation - EnemyLocation;
	const bool bNormOk = Diff.Normalize(BombBlackboardNormalizeTolerance);
	if (false == bNormOk)
	{
		return false;
	}

	OutDirection = Diff;
	return true;
}
