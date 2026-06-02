#include "Characters/AI/CBTService_Melee.h"

#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Utilities/CLog.h"
#include "Utilities/YJJHelpers.h"

UCBTService_Melee::UCBTService_Melee()
{
	NodeName = TEXT("BTService Melee");
}

void UCBTService_Melee::TickCharacterAi(UBehaviorTreeComponent& OwnerComp)
{
	ResolveOwnerAndControlledCharacter(OwnerComp);

	if (false == TryRefreshCurStateFromStateComponent())
	{
		// Possess/StateComp 미준비 — BT 틱마다 흔한 경로라 로그 없음.
		return;
	}

	ApplyBlackboardTargetMirror(OwnerComp);

	ACCommonCharacter* bbTarget = ResolveBlackboardTargetCharacter(OwnerComp);
	if (IsValid(bbTarget))
	{
		if (false == IsValid(ControlledCharacterRef))
		{
			CLog::Log(TEXT("[UCBTService_Melee] 블랙보드 타겟은 있는데 ControlledCharacterRef 없음."));
			return;
		}

		const float distCmFloat = ControlledCharacterRef->GetDistanceTo(bbTarget);
		const double distCmAsDouble = static_cast<double>(distCmFloat);

		if (distCmAsDouble <= ActionDistance)
		{
			PushBehaviorBlackboardRawByte(OwnerComp, BehaviorByte_InMeleeRange);
			return;
		}

		const bool bIdleOrRise =
			(CurState == CEStateType::Idle) || (CurState == CEStateType::Rise);
		if (false == bIdleOrRise)
		{
			return;
		}

		UCStateComponent* const stateComp =
			YJJHelpers::GetComponent<UCStateComponent>(ControlledCharacterRef.Get());
		if (false == IsValid(stateComp))
		{
			CLog::Log(TEXT("[UCBTService_Melee] StateComp 없음 — Idle/Rise 접근 분기 생략."));
			return;
		}

		if (false == stateComp->IsAction())
		{
			PushBehaviorBlackboardRawByte(OwnerComp, BehaviorByte_OutOfRangeIdleOrRise);
		}

		return;
	}

	if (false == bTraceWhenMissingBlackboardTarget)
	{
		PushBehaviorBlackboardRawByte(OwnerComp, BehaviorByte_TraceMissed);
		return;
	}

	const bool bTraceFound = TryTraceAcquireTarget(OwnerComp);
	if (true == bTraceFound)
	{
		// TryTraceAcquireTarget 이 레거시 SetTarget + BB 기록까지 처리 — Patrol 분기 생략.
		return;
	}

	PushBehaviorBlackboardRawByte(OwnerComp, BehaviorByte_TraceMissed);
}
