#include "Characters/AI/CBTService_DragonBoss.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "Characters/Animals/CAnimal.h"
#include "Characters/Animals/Dragon/CDragon.h"
#include "Characters/CCommonCharacter.h"
#include "Commons/CYJJBlueprintLibrary.h"
#include "Components/CStateComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Internationalization/Text.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
	static bool BossTryAppendUniqueFresh(TArray<ACCommonCharacter*>& InOutSeen, ACCommonCharacter* CandidateLocal)
	{
		if ((false == IsValid(CandidateLocal)) || InOutSeen.Contains(CandidateLocal))
		{
			return false;
		}

		InOutSeen.Add(CandidateLocal);

		return true;
	}

	static FVector BossActorLocationSafe(const AActor* ActorLocal)
	{
		if ((false == IsValid(ActorLocal)))
		{
			return FVector::ZeroVector;
		}

		return ActorLocal->GetActorLocation();
	}
} // namespace

UCBTService_DragonBoss::UCBTService_DragonBoss()
{
	NodeName = TEXT("BTService_DragonBoss");
	bCreateNodeInstance = true;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	bNotifyTick = true;

	Interval = 0.f;
	RandomDeviation = 0.f;
}

void UCBTService_DragonBoss::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	const UBlackboardData* bbAssetLocal = Asset.BlackboardAsset;

	if (nullptr == bbAssetLocal)
	{
		return;
	}

	BehaviorKey.ResolveSelectedKey(*bbAssetLocal);
	TargetKey.ResolveSelectedKey(*bbAssetLocal);
	DistanceTypeKey.ResolveSelectedKey(*bbAssetLocal);
}

void UCBTService_DragonBoss::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	OwningAi = OwnerComp.GetAIOwner();
	HittedUniqueAccumulator.Reset();

	ACDragon* dragonScratch = nullptr;
	(void)ResolveControlledDragon(OwnerComp, dragonScratch);
	ResolveCurStateFromPawn(dragonScratch);
	RefreshBlackboardTargetMirror(OwnerComp);
}

void UCBTService_DragonBoss::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	OwningAi = nullptr;
	CachedDragon = nullptr;
	BlackboardTargetMirror = nullptr;
	HittedUniqueAccumulator.Reset();
}

void UCBTService_DragonBoss::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	TickDragonBossAi(OwnerComp, DeltaSeconds);
}

void UCBTService_DragonBoss::TickDragonBossAi(UBehaviorTreeComponent& OwnerComp,
	float DeltaSeconds)
{
	ACDragon* dragonLocal = nullptr;

	if ((false == ResolveControlledDragon(OwnerComp, dragonLocal)) || (false == IsValid(dragonLocal)))
	{
		// Possess 미완·데디 스폰 직후 등에서는 흔히 nullptr — 블프와 같이 무음 종료(BT 다른 노드 처리).
		return;
	}

	(void) ResolveCurStateFromPawn(dragonLocal);
	CachedDragon = dragonLocal;

	RefreshBlackboardTargetMirror(OwnerComp);

	if ((false == ShouldTickTargetAndFacingForCurrentState()))
	{
		return;
	}

	const bool bbHasTargetLocal = IsValid(BlackboardTargetMirror.Get());

	if (bbHasTargetLocal)
	{
		RotateDragonTowardBbTarget(dragonLocal, DeltaSeconds);
		EvaluateDistanceBandsAndBehavior(dragonLocal, OwnerComp);
	}
	else
	{
		TraceAndResolveCombatTarget(dragonLocal, OwnerComp);
		RefreshBlackboardTargetMirror(OwnerComp);

		if (true == IsValid(BlackboardTargetMirror.Get()))
		{
			RotateDragonTowardBbTarget(dragonLocal, DeltaSeconds);
			EvaluateDistanceBandsAndBehavior(dragonLocal, OwnerComp);
		}
	}
}

bool UCBTService_DragonBoss::ResolveControlledDragon(UBehaviorTreeComponent& OwnerComp,
	ACDragon*& OutDragon)
{
	AAIController* const aiOwner = OwnerComp.GetAIOwner();

	if (false == IsValid(aiOwner))
	{
		OutDragon = nullptr;
		return false;
	}

	OwningAi = aiOwner;

	APawn* const pawnScratch = aiOwner->GetPawn();

	OutDragon = Cast<ACDragon>(pawnScratch);

	return IsValid(OutDragon);
}

bool UCBTService_DragonBoss::ResolveCurStateFromPawn(ACDragon* Dragon)
{
	CurStateTyped = CEStateType::Idle;

	if (false == IsValid(Dragon))
	{
		return false;
	}

	UCStateComponent* const stateComp = Dragon->FindComponentByClass<UCStateComponent>();

	if (false == IsValid(stateComp))
	{
		return true;
	}

	CurStateTyped = stateComp->GetCurMode();

	return true;
}

void UCBTService_DragonBoss::RefreshBlackboardTargetMirror(UBehaviorTreeComponent& OwnerComp)
{
	BlackboardTargetMirror = nullptr;

	if (false == TargetKey.IsSet())
	{
		return;
	}

	UBlackboardComponent* const bb = OwnerComp.GetBlackboardComponent();

	if (false == IsValid(bb))
	{
		return;
	}

	UObject* rawTarget = bb->GetValueAsObject(TargetKey.SelectedKeyName);
	BlackboardTargetMirror = Cast<ACCommonCharacter>(rawTarget);
}

bool UCBTService_DragonBoss::ShouldTickTargetAndFacingForCurrentState() const
{
	switch (CurStateTyped)
	{
	case CEStateType::Idle:
	case CEStateType::Act:
	case CEStateType::CombatHitted:
	case CEStateType::Flying:
	case CEStateType::Fall:
		return true;
	default:
		return false;
	}
}

void UCBTService_DragonBoss::RotateDragonTowardBbTarget(ACDragon* Dragon,
	float DeltaSeconds)
{
	ACCommonCharacter* const bbTargetCharacter = BlackboardTargetMirror.Get();

	if ((false == IsValid(Dragon)) || (false == IsValid(bbTargetCharacter)))
	{
		return;
	}

	const FVector startLocal = BossActorLocationSafe(Dragon);
	const FVector targetLocal = BossActorLocationSafe(bbTargetCharacter);

	const FRotator currentRotLocal = Dragon->GetActorRotation();

	const FRotator targetLookLocal = UKismetMathLibrary::FindLookAtRotation(startLocal, targetLocal);

	const bool yawNearlySameLocal =
		FMath::IsNearlyEqual(static_cast<double>(currentRotLocal.Yaw), static_cast<double>(targetLookLocal.Yaw),
			static_cast<double>(RotationYawEpsilonDegrees));

	if (true == yawNearlySameLocal)
	{
		Dragon->bIsRotating = false;
		return;
	}

	// 레거시 분할 핀이 Roll/Pitch 를 보존하는 형태였으므로 하늘 패턴에서 기울기가 튀지 않도록 Yaw 만 목표값으로 두고 나머지 축은 유지한다.
	const FRotator faceTargetPreservePitchRollExceptYawLocal(currentRotLocal.Pitch, targetLookLocal.Yaw,
		currentRotLocal.Roll);

	const FRotator interpolatedLocal =
		UKismetMathLibrary::RInterpTo(currentRotLocal, faceTargetPreservePitchRollExceptYawLocal, DeltaSeconds,
			RotationInterpSpeed);

	const bool bRotationAppliedLocal = Dragon->K2_SetActorRotation(interpolatedLocal, false /* bTeleport */);
	Dragon->bIsRotating = bRotationAppliedLocal;
}

void UCBTService_DragonBoss::EvaluateDistanceBandsAndBehavior(ACDragon* Dragon,
	UBehaviorTreeComponent& OwnerComp)
{
	ACCommonCharacter* const tgt = BlackboardTargetMirror.Get();

	if ((false == IsValid(Dragon)) || (false == IsValid(tgt)))
	{
		return;
	}

	const double distSqScratch = static_cast<double>(tgt->GetSquaredDistanceTo(Dragon));

	uint8 distanceByteScratch = DistanceTypeBB_Avoid;
	uint8 bossBehScratch = BossBehavior_SkillAvoidBandByte;

	if (distSqScratch > DistanceSquaredThreshold_Approach)
	{
		distanceByteScratch = DistanceTypeBB_Approach;
		bossBehScratch = BossBehavior_ApproachBandByte;
	}
	else if (distSqScratch > DistanceSquaredThreshold_Skill)
	{
		distanceByteScratch = DistanceTypeBB_Skill;
		bossBehScratch = BossBehavior_SkillAvoidBandByte;
	}
	else if (distSqScratch > DistanceSquaredThreshold_Action)
	{
		distanceByteScratch = DistanceTypeBB_Action;
		bossBehScratch = BossBehavior_ActionBandByte;
	}
	else
	{
		distanceByteScratch = DistanceTypeBB_Avoid;
		bossBehScratch = BossBehavior_SkillAvoidBandByte;
	}

	WriteBlackboardEnumIfSet(OwnerComp, DistanceTypeKey, distanceByteScratch);

	PushBossBehavior(Dragon, OwnerComp, bossBehScratch);
}

void UCBTService_DragonBoss::TraceAndResolveCombatTarget(ACDragon* Dragon,
	UBehaviorTreeComponent& OwnerComp)
{
	UWorld* const worldLocal = IsValid(Dragon) ? Dragon->GetWorld() : nullptr;

	if ((false == IsValid(worldLocal)) || (false == TargetKey.IsSet()))
	{
		return;
	}

	const FVector traceOriginLocal = BossActorLocationSafe(Dragon);

	TArray<TEnumAsByte<EObjectTypeQuery>> typesLocal;

	typesLocal.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ignoreLocal;

	AppendIgnoreActors(Dragon, ignoreLocal);

	TArray<FHitResult> hitsScratch;

	const bool tracedLocal = UKismetSystemLibrary::SphereTraceMultiForObjects(
		worldLocal,
		traceOriginLocal,
		traceOriginLocal,
		SphereTraceRadius,
		typesLocal,
		false /* traceComplex */,
		ignoreLocal,
		EDrawDebugTrace::None,
		hitsScratch,
		true /* ignoreSelf */,
		FLinearColor::Red,
		FLinearColor::Green,
		5.f);

	if ((false == tracedLocal) || hitsScratch.Num() <= 0)
	{
		return;
	}

	UBlackboardComponent* bbCompLocal = OwnerComp.GetBlackboardComponent();

	if ((false == IsValid(bbCompLocal)) || (false == IsValid(Dragon)))
	{
		return;
	}

	const int32 hitCountLocal = hitsScratch.Num();

	for (int32 hitIdxLocal = 0; hitIdxLocal < hitCountLocal; ++hitIdxLocal)
	{
		AActor* const hitActor = hitsScratch[hitIdxLocal].GetActor();

		ACCommonCharacter* const asCharScratch = Cast<ACCommonCharacter>(hitActor);

		if ((false == IsValid(asCharScratch)) || (asCharScratch == Dragon))
		{
			continue;
		}

		const bool sameGroupScratch = UCYJJBlueprintLibrary::BFCompat_IsSameGroup(Dragon, asCharScratch);

		if (true == sameGroupScratch)
		{
			continue;
		}

		const bool freshScratch = BossTryAppendUniqueFresh(HittedUniqueAccumulator, asCharScratch);

		if (false == freshScratch)
		{
			continue;
		}

		bbCompLocal->SetValueAsObject(TargetKey.SelectedKeyName, asCharScratch);
		break;
	}
}

void UCBTService_DragonBoss::AppendIgnoreActors(ACDragon* Dragon, TArray<AActor*>& InOutActors) const
{
	if (false == IsValid(Dragon))
	{
		return;
	}

	InOutActors.Add(Dragon);

	if (AActor* const dragonWeaponLocal = Dragon->DragonWeapon.Get())
	{
		InOutActors.Add(dragonWeaponLocal);
	}

	ACAnimal* const animalAncestor = Cast<ACAnimal>(Dragon);

	if (nullptr != animalAncestor)
	{
		if (AActor* animalWeaponScratch = animalAncestor->AnimalWeapon.Get())
		{
			InOutActors.Add(animalWeaponScratch);
		}
	}
}

void UCBTService_DragonBoss::WriteBlackboardEnumIfSet(UBehaviorTreeComponent& OwnerComp,
	const FBlackboardKeySelector& Key, const uint8 Value)
{
	if (false == Key.IsSet())
	{
		return;
	}

	UBlackboardComponent* const bb = OwnerComp.GetBlackboardComponent();

	if (false == IsValid(bb))
	{
		return;
	}

	bb->SetValueAsEnum(Key.SelectedKeyName, Value);
}

void UCBTService_DragonBoss::PushBossBehavior(ACDragon* Dragon,
	UBehaviorTreeComponent& OwnerComp,
	const uint8 BehaviorByte)
{
	WriteBlackboardEnumIfSet(OwnerComp, BehaviorKey, BehaviorByte);
	TryWriteStateDebugText(Dragon, BehaviorByte);
}

void UCBTService_DragonBoss::TryWriteStateDebugText(ACCommonCharacter* TextSourceCharacter,
	uint8 BossBehaviorByte)
{
	if ((false == bUpdateStateTextRender) || (false == IsValid(TextSourceCharacter)))
	{
		return;
	}

	UTextRenderComponent* const renderLocal = TextSourceCharacter->GetYJJStateTextRenderComponent();

	if (false == IsValid(renderLocal))
	{
		return;
	}

	const FString labelLocal =
		FString::Printf(TEXT("DragonBoss:%u"), static_cast<uint32>(BossBehaviorByte));
	renderLocal->SetText(FText::FromString(labelLocal));
}
