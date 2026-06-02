#include "Weapons/CMagicFireBall.h"

#include "Weapons/CSkillFireBall.h"
#include "Utilities/CLog.h"
#include "Characters/CCommonCharacter.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"

const FName ACMagicFireBall::FireBallAiBlackboardTargetKey(TEXT("Target"));

void ACMagicFireBall::DoAction_Implementation(CEAttackType const InAttackType, int32 const InSkillIndex)
{
	(void)InAttackType;
	(void)InSkillIndex;

	Super::DoAction_Implementation(InAttackType, InSkillIndex);

	const int32 RowCountScratch = DoActionDatas.Num();
	const bool HasRowsScratch = RowCountScratch > 0;
	const bool IdleOrRidingScratch = true == Magic_IsOwnerIdle() || true == Magic_IsOwnerRiding();

	if ((false == HasRowsScratch) || (false == IdleOrRidingScratch))
	{
		return;
	}

	Magic_SetOwnerStateAction();
	PlayAction(DoActionDatas, 0);
}

void ACMagicFireBall::Begin_DoAction_Implementation(CEAttackType const InAttackType)
{
	Super::Begin_DoAction_Implementation(InAttackType);

	(void)InAttackType;

	FireBallDirectionScratch = FVector::ZeroVector;

	if (true == IsValid(Controller.Get()))
	{
		FRotator const ControlRotScratch = Controller->GetControlRotation();
		FireBallDirectionScratch = UKismetMathLibrary::GetForwardVector(ControlRotScratch);
	}
	else
	{
		FVector FromBbScratch = FVector::ZeroVector;
		bool const BbOkScratch = FireBall_TryResolveDirectionFromBlackboardScratch(FromBbScratch);
		if (true == BbOkScratch)
		{
			FireBallDirectionScratch = FromBbScratch;
		}
		else
		{
			CLog::Log(FString::Printf(
				TEXT("[MagicFireBall] Controller 없고 블랙보드 Target 방향 실패 — 폴백 0 벡터 — %s"),
				*GetNameSafe(this)));
		}
	}

	ACharacter* AsCharacterScratch = Cast<ACharacter>(Character.Get());

	if (false == IsValid(AsCharacterScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicFireBall] Character 없음 — 스폰 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	USkeletalMeshComponent* MeshScratch = AsCharacterScratch->GetMesh();

	if (false == IsValid(MeshScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicFireBall] Mesh 없음 — 스폰 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	FVector const SpawnLocationScratch = MeshScratch->GetSocketLocation(FireBallSpawnSocketName);

	UWorld* WorldScratch = GetWorld();

	if (false == IsValid(WorldScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicFireBall] World 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	if (nullptr == SkillFireBallActorClass.Get())
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicFireBall] SkillFireBallActorClass 미설정 — 블프 `ACSkillFireBall` 기본 클래스 지정 — %s"),
			*GetNameSafe(this)));
		return;
	}

	FRotator const SpawnRotationScratch =
		FireBallDirectionScratch.IsNearlyZero(0.0001f)
			? MeshScratch->GetSocketRotation(FireBallSpawnSocketName)
			: UKismetMathLibrary::Conv_VectorToRotator(FireBallDirectionScratch);

	FTransform SpawnTransformScratch(SpawnRotationScratch, SpawnLocationScratch, FVector::OneVector);

	FActorSpawnParameters ParamsScratch;

	ParamsScratch.Owner = this;

	ParamsScratch.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACSkillFireBall* SpawnedScratch =
		WorldScratch->SpawnActor<ACSkillFireBall>(SkillFireBallActorClass, SpawnTransformScratch, ParamsScratch);

	if (false == IsValid(SpawnedScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicFireBall] SpawnActor 실패 — Class=%s — %s"),
			*GetNameSafe(SkillFireBallActorClass.Get()),
			*GetNameSafe(this)));
		return;
	}

	SpawnedScratch->Shoot(FireBallDirectionScratch);
}

bool ACMagicFireBall::FireBall_TryResolveDirectionFromBlackboardScratch(FVector& OutDirectionScratch) const
{
	ACCommonCharacter* OwnerScratch = Character.Get();
	if (false == IsValid(OwnerScratch))
	{
		return false;
	}

	UBlackboardComponent* BbScratch = UAIBlueprintHelperLibrary::GetBlackboard(OwnerScratch);

	if (false == IsValid(BbScratch))
	{
		return false;
	}

	UObject* TargetRawScratch = BbScratch->GetValueAsObject(FireBallAiBlackboardTargetKey);
	AActor* TargetActorScratch = Cast<AActor>(TargetRawScratch);

	if (false == IsValid(TargetActorScratch))
	{
		return false;
	}

	FVector DiffScratch = TargetActorScratch->GetActorLocation() - OwnerScratch->GetActorLocation();
	bool const NormOkScratch = DiffScratch.Normalize(FireBallBlackboardNormalizeToleranceScratch);

	if (false == NormOkScratch)
	{
		return false;
	}

	OutDirectionScratch = DiffScratch;

	return true;
}
