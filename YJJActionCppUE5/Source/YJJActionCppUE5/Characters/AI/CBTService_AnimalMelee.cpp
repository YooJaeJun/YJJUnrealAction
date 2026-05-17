#include "Characters/AI/CBTService_AnimalMelee.h"

#include "Characters/CCommonCharacter.h"
#include "Utilities/CLog.h"

UCBTService_AnimalMelee::UCBTService_AnimalMelee()
{
	NodeName = TEXT("BT Service (Animal Melee)");
}

void UCBTService_AnimalMelee::TickCharacterAi(UBehaviorTreeComponent& OwnerComp)
{
	ResolveOwnerAndControlledCharacter(OwnerComp);

	if (false == TryRefreshCurStateFromStateComponent())
	{
		// 기본 클래스와 동일: 상태 미확보는 틱마다 발생할 수 있어 무음 종료(Tick 상태 스팸 방지).
		return;
	}

	ApplyBlackboardTargetMirror(OwnerComp);

	ACCommonCharacter* bbTarget = ResolveBlackboardTargetCharacter(OwnerComp);
	if (IsValid(bbTarget))
	{
		if (false == IsValid(ControlledCharacterRef))
		{
			CLog::Log(FString(
				TEXT("BTService_AnimalMelee: 블랙보드 타겟은 있는데 ControlledCharacterRef 가 없음(포제스/폰 종류 검토).")));
			return;
		}

		const float distCmFloat = ControlledCharacterRef->GetDistanceTo(bbTarget);
		const double distCmAsDouble = static_cast<double>(distCmFloat);

		if (distCmAsDouble <= ActionDistance)
		{
			// 레거시: 접근 무시하고 즉시 근거리 행동으로 Behavior 전환.
			PushBehaviorBlackboardRawByte(OwnerComp, BehaviorByte_InMeleeRange);
			return;
		}

		// 레거시 스윕 블프: Idle(NewEnumerator0)·Rise(NewEnumerator5) — `CEStateType` 표시값 순서 동기화 전제.
		const bool bIdleOrRise =
			(CurState == CEStateType::Idle) || (CurState == CEStateType::Rise);
		if (true == bIdleOrRise)
		{
			PushBehaviorBlackboardRawByte(OwnerComp, BehaviorByte_OutOfRangeIdleOrRise);
		}

		return;
	}

	// 레거시: 블프는 무조건 트레이스 분기지만, 네이티브 공통 플래그로 상위 클래스와 같은 스위치를 공유한다.
	if (false == bTraceWhenMissingBlackboardTarget)
	{
		return;
	}

	const bool bTraceFound = TryTraceAcquireTarget(OwnerComp);
	if (false == bTraceFound)
	{
		PushBehaviorBlackboardRawByte(OwnerComp, BehaviorByte_TraceMissed);
	}
}
