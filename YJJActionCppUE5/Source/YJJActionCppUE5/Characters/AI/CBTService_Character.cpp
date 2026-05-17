#include "Characters/AI/CBTService_Character.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/CStateComponent.h"
#include "Engine/EngineTypes.h"
#include "Global.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utilities/CLog.h"
#include "Utilities/YJJHelpers.h"

UCBTService_Character::UCBTService_Character()
{
	NodeName = TEXT("BTService_Character");
	bCreateNodeInstance = true;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	bNotifyTick = true;
	Interval = 0.0f;
	RandomDeviation = 0.f;
}

void UCBTService_Character::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	const UBlackboardData* bbAsset = Asset.BlackboardAsset;
	if (nullptr == bbAsset)
	{
		return;
	}

	BehaviorKey.ResolveSelectedKey(*bbAsset);
	TargetKey.ResolveSelectedKey(*bbAsset);
}

void UCBTService_Character::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	ResolveOwnerAndControlledCharacter(OwnerComp);
	TryRefreshCurStateFromStateComponent();
	ApplyBlackboardTargetMirror(OwnerComp);
}

void UCBTService_Character::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	OwningControllerRef = nullptr;
	ControlledCharacterRef = nullptr;
	BlackboardTargetMirror = nullptr;
}

void UCBTService_Character::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	TickCharacterAi(OwnerComp);
}

void UCBTService_Character::TickCharacterAi(UBehaviorTreeComponent& OwnerComp)
{
	ResolveOwnerAndControlledCharacter(OwnerComp);

	if (false == TryRefreshCurStateFromStateComponent())
	{
		// 상태 컴포넌트가 없거나 Possess 미완 같은 경우는 다른 BT 노드와 동일하게 틱마다 무음 조기 종료.
		return;
	}

	ApplyBlackboardTargetMirror(OwnerComp);

	if (nullptr == BlackboardTargetMirror.Get())
	{
		if (true == bTraceWhenMissingBlackboardTarget)
		{
			const bool bFound = TryTraceAcquireTarget(OwnerComp);
			if (true == bFound)
			{
				ApplyBlackboardTargetMirror(OwnerComp);
			}
		}
	}

	TickStateBranches(OwnerComp);
}

void UCBTService_Character::ResolveOwnerAndControlledCharacter(UBehaviorTreeComponent& OwnerComp)
{
	AAIController* aiOwner = OwnerComp.GetAIOwner();
	OwningControllerRef = aiOwner;

	if (nullptr == aiOwner)
	{
		ControlledCharacterRef = nullptr;
		return;
	}

	APawn* const pawn = aiOwner->GetPawn();
	ControlledCharacterRef = Cast<ACCommonCharacter>(pawn);
}

bool UCBTService_Character::TryRefreshCurStateFromStateComponent()
{
	if (false == IsValid(ControlledCharacterRef))
	{
		return false;
	}

	TObjectPtr<UCStateComponent> stateComp =
		YJJHelpers::GetComponent<UCStateComponent>(ControlledCharacterRef.Get());
	if (false == IsValid(stateComp))
	{
		return false;
	}

	CurState = stateComp->GetCurMode();

	return true;
}

void UCBTService_Character::ApplyBlackboardTargetMirror(UBehaviorTreeComponent& OwnerComp)
{
	BlackboardTargetMirror = nullptr;

	if (false == TargetKey.IsSet())
	{
		return;
	}

	UBlackboardComponent* const blackboardComp = OwnerComp.GetBlackboardComponent();
	if (nullptr == blackboardComp)
	{
		return;
	}

	UObject* const rawTarget = blackboardComp->GetValueAsObject(TargetKey.SelectedKeyName);
	BlackboardTargetMirror = Cast<ACCommonCharacter>(rawTarget);
}

ACCommonCharacter*
UCBTService_Character::ResolveBlackboardTargetCharacter(UBehaviorTreeComponent& OwnerComp) const
{
	const UBlackboardComponent* const bb = OwnerComp.GetBlackboardComponent();
	if ((nullptr == bb) || (false == TargetKey.IsSet()))
	{
		return nullptr;
	}

	return Cast<ACCommonCharacter>(bb->GetValueAsObject(TargetKey.SelectedKeyName));
}

void UCBTService_Character::TryWriteBlackboardTargetObject(UBehaviorTreeComponent& OwnerComp,
	ACCommonCharacter* InTarget)
{
	if ((nullptr == InTarget) || (false == TargetKey.IsSet()))
	{
		return;
	}

	UBlackboardComponent* const bb = OwnerComp.GetBlackboardComponent();
	if (nullptr == bb)
	{
		return;
	}

	bb->SetValueAsObject(TargetKey.SelectedKeyName, InTarget);
}

void UCBTService_Character::PushBehaviorBlackboard(UBehaviorTreeComponent& OwnerComp,
	CEBehaviorType InBehaviorType)
{
	if (false == bIsSetBehavior)
	{
		return;
	}

	if (false == BehaviorKey.IsSet())
	{
		return;
	}

	UBlackboardComponent* const bb = OwnerComp.GetBlackboardComponent();
	if (nullptr == bb)
	{
		CLog::Log(FString(
			TEXT("BTService_Character: Blackboard 컴포넌트가 없어 BehaviorKey 에 Enum 을 기록하지 못함.")));
		return;
	}

	const uint8 asByte = static_cast<uint8>(InBehaviorType);
	bb->SetValueAsEnum(BehaviorKey.SelectedKeyName, asByte);
}

void UCBTService_Character::PushBehaviorBlackboardRawByte(UBehaviorTreeComponent& OwnerComp,
	uint8 InBehaviorByte)
{
	if (false == bIsSetBehavior)
	{
		return;
	}

	if (false == BehaviorKey.IsSet())
	{
		return;
	}

	UBlackboardComponent* const bb = OwnerComp.GetBlackboardComponent();
	if (nullptr == bb)
	{
		CLog::Log(FString(
			TEXT("BTService_Character: Blackboard 컴포넌트가 없어 BehaviorKey 에 원시 바이트를 기록하지 못함.")));
		return;
	}

	bb->SetValueAsEnum(BehaviorKey.SelectedKeyName, InBehaviorByte);
}

bool UCBTService_Character::SetAITarget(UBehaviorTreeComponent& OwnerComp, ACCommonCharacter* InTarget)
{
	(void)OwnerComp;

	if (nullptr == InTarget)
	{
		return false;
	}

	if (false == IsValid(ControlledCharacterRef))
	{
		CLog::Log(FString(TEXT("BTService_Character: SetAITarget 호출 시 유효한 Controlled 가 없음.")));
		return false;
	}

	UCCharacterInfoComponent* info =
		YJJHelpers::GetComponent<UCCharacterInfoComponent>(ControlledCharacterRef.Get());

	if ((nullptr != info) && (info->IsSameGroup(TWeakObjectPtr<ACCommonCharacter>(InTarget))))
	{
		return false;
	}

	BlackboardTargetMirror = InTarget;
	ControlledCharacterRef->NextMovingPoint = InTarget->GetActorLocation();

	return true;
}

void UCBTService_Character::RunIdleEquipActTargetFlow(UBehaviorTreeComponent& OwnerComp)
{
	ACCommonCharacter* const bbTarget = ResolveBlackboardTargetCharacter(OwnerComp);
	if (false == IsValid(bbTarget))
	{
		return;
	}

	(void) SetAITarget(OwnerComp, bbTarget);
}

void UCBTService_Character::RunCombatHittedFlow(UBehaviorTreeComponent& OwnerComp)
{
	if (false == IsValid(ControlledCharacterRef))
	{
		return;
	}

	ACCommonCharacter* const attacker = ControlledCharacterRef->Attacker;
	if (false == IsValid(attacker))
	{
		return;
	}

	TryWriteBlackboardTargetObject(OwnerComp, attacker);

	(void) SetAITarget(OwnerComp, attacker);

	PushBehaviorBlackboard(OwnerComp, CEBehaviorType::CombatHitted);
}

void UCBTService_Character::RunRiseFlow(UBehaviorTreeComponent& OwnerComp)
{
	PushBehaviorBlackboard(OwnerComp, CEBehaviorType::Rise);

	ACCommonCharacter* const bbTarget = ResolveBlackboardTargetCharacter(OwnerComp);
	if (false == IsValid(bbTarget))
	{
		return;
	}

	(void) SetAITarget(OwnerComp, bbTarget);
}

bool UCBTService_Character::TryTraceAcquireTarget(UBehaviorTreeComponent& OwnerComp)
{
	if (false == IsValid(ControlledCharacterRef))
	{
		return false;
	}

	if (false == TargetKey.IsSet())
	{
		return false;
	}

	if (nullptr == ControlledCharacterRef->GetWorld())
	{
		CLog::Log(FString(TEXT("BTService_Character: Trace 시 World 없음.")));
		return false;
	}

	const FVector traceOrigin = ControlledCharacterRef->GetActorLocation();

	TArray<TEnumAsByte<EObjectTypeQuery>> objectParams;
	objectParams.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	objectParams.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	TArray<AActor*> actorsToIgnore;
	const EDrawDebugTrace::Type dbg = EDrawDebugTrace::None;
	TArray<FHitResult> hits;
	constexpr bool traceComplex = false;

	constexpr bool ignoreSelfTrace = true;

	UWorld* const world = ControlledCharacterRef->GetWorld();
	const bool traced = UKismetSystemLibrary::SphereTraceMultiForObjects(
		world,
		traceOrigin,
		traceOrigin,
		SphereTraceRadius,
		objectParams,
		traceComplex,
		actorsToIgnore,
		dbg,
		hits,
		ignoreSelfTrace,
		FLinearColor::Red,
		FLinearColor::Green,
		5.0f);

	if ((false == traced) || hits.Num() == 0)
	{
		return false;
	}

	const FHitResult& picked = hits[0];

	AActor* hitActor = picked.GetActor();
	ACCommonCharacter* const asChar = Cast<ACCommonCharacter>(hitActor);
	if ((nullptr == asChar) || (asChar == ControlledCharacterRef))
	{
		return false;
	}

	if (false == SetAITarget(OwnerComp, asChar))
	{
		return false;
	}

	TryWriteBlackboardTargetObject(OwnerComp, asChar);

	return true;
}

void UCBTService_Character::TickStateBranches(UBehaviorTreeComponent& OwnerComp)
{
	const CEStateType mode = CurState;

	switch (mode)
	{
	case CEStateType::Idle:
	case CEStateType::Equip:
	case CEStateType::Act:
		RunIdleEquipActTargetFlow(OwnerComp);
		break;

	case CEStateType::CombatHitted:
		RunCombatHittedFlow(OwnerComp);
		break;

	case CEStateType::Dead:
		PushBehaviorBlackboard(OwnerComp, CEBehaviorType::Dead);
		break;

	case CEStateType::Flying:
		PushBehaviorBlackboard(OwnerComp, CEBehaviorType::Flying);
		break;

	case CEStateType::Fall:
		PushBehaviorBlackboard(OwnerComp, CEBehaviorType::Falling);
		break;

	case CEStateType::HitAir:
		PushBehaviorBlackboard(OwnerComp, CEBehaviorType::HitAir);
		break;

	case CEStateType::DownFlying:
		PushBehaviorBlackboard(OwnerComp, CEBehaviorType::DownFlying);
		break;

	case CEStateType::Land:
		PushBehaviorBlackboard(OwnerComp, CEBehaviorType::DownLand);
		break;

	case CEStateType::Rise:
		RunRiseFlow(OwnerComp);
		break;

	case CEStateType::Riding:
		PushBehaviorBlackboard(OwnerComp, CEBehaviorType::Riding);
		break;

	case CEStateType::Avoid:
		PushBehaviorBlackboard(OwnerComp, CEBehaviorType::Avoid);
		break;

	case CEStateType::Cinematic:
	case CEStateType::Dash:
	case CEStateType::Parkour:
	case CEStateType::Groggy:
	case CEStateType::Legacy_MaxSlot:
	case CEStateType::Max:
	default:
		break;
	}
}
