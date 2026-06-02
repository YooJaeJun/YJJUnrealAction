#include "Characters/AI/CBTService_Magic.h"

#include "AIController.h"
#include "Characters/CCommonCharacter.h"
#include "Utilities/CLog.h"

UCBTService_Magic::UCBTService_Magic()
{
	NodeName = TEXT("BTService Magic");
}

void UCBTService_Magic::TickCharacterAi(UBehaviorTreeComponent& OwnerComp)
{
	ResolveOwnerAndControlledCharacter(OwnerComp);

	if (false == TryRefreshCurStateFromStateComponent())
	{
		// Possess/StateComp 미준비 — BT 틱마다 흔한 경로라 로그 없음.
		return;
	}

	ApplyBlackboardTargetMirror(OwnerComp);

	AAIController* const aiController = OwningControllerRef.Get();
	ACCommonCharacter* const bbTarget = ResolveBlackboardTargetCharacter(OwnerComp);

	if (false == IsValid(bbTarget))
	{
		PushBehaviorBlackboardRawByte(OwnerComp, BehaviorByte_NoTarget);

		if (IsValid(aiController))
		{
			aiController->ClearFocus(EAIFocusPriority::Gameplay);
		}

		return;
	}

	if (IsValid(aiController))
	{
		aiController->SetFocus(bbTarget, EAIFocusPriority::Gameplay);
	}

	if (false == IsValid(ControlledCharacterRef))
	{
		CLog::Log(FString::Printf(
			TEXT("[UCBTService_Magic] 타겟은 있는데 ControlledCharacterRef 없음 — Controller=%s"),
			*GetNameSafe(OwningControllerRef.Get())));
		return;
	}

	const float distCmFloat = ControlledCharacterRef->GetDistanceTo(bbTarget);
	const double distCmAsDouble = static_cast<double>(distCmFloat);

	if (distCmAsDouble <= AvoidDistance)
	{
		PushBehaviorBlackboardRawByte(OwnerComp, BehaviorByte_WithinAvoidDistance);
	}
	else
	{
		PushBehaviorBlackboardRawByte(OwnerComp, BehaviorByte_BeyondAvoidDistance);
	}
}
