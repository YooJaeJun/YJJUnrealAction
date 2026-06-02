#include "Characters/AI/CBTTask_Act.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Commons/CEnums.h"
#include "Components/CMagicComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CWeaponComponent.h"
#include "Global.h"
#include "Utilities/YJJHelpers.h"
#include "Weapons/CMagic.h"
#include "Weapons/CWeapon.h"

namespace
{
	static bool Act_IsWeaponActionTerminal(UCWeaponComponent* WeaponComp, UCStateComponent* StateComp)
	{
		if ((false == IsValid(WeaponComp)) || (false == IsValid(StateComp)))
		{
			return false;
		}

		ACWeapon* const mainWeapon = WeaponComp->GetMainWeapon();
		if (false == IsValid(mainWeapon))
		{
			return false;
		}

		const bool notInAction = false == mainWeapon->InAction;
		const bool idle = StateComp->IsIdle();
		return notInAction && idle;
	}

	static bool Act_IsMagicActionTerminal(UCMagicComponent* MagicComp, UCStateComponent* StateComp)
	{
		if ((false == IsValid(MagicComp)) || (false == IsValid(StateComp)))
		{
			return false;
		}

		ACMagic* const activeMagic = MagicComp->ActiveMagic.Get();
		if (false == IsValid(activeMagic))
		{
			return false;
		}

		const bool notInAction = false == activeMagic->InAction;
		const bool idle = StateComp->IsIdle();
		return notInAction && idle;
	}

	static void Act_DispatchDoAction(UCWeaponComponent* WeaponComp, UCMagicComponent* MagicComp)
	{
		// 마법 장착 중이면 MainWeapon 대신 ActiveMagic 경로로 공격을 시작한다.
		if (IsValid(MagicComp) && (false == MagicComp->IsUnarmed()))
		{
			MagicComp->DoAction(CEAttackType::Common, 0);
			return;
		}

		if (IsValid(WeaponComp))
		{
			WeaponComp->LegacyBp_DispatchMain_DoAction(CEAttackType::Common, 0);
		}
	}

	static void Act_ApplyAbortWeaponCleanup(UCWeaponComponent* WeaponComp)
	{
		if (false == IsValid(WeaponComp))
		{
			return;
		}

		ACWeapon* const mainWeapon = WeaponComp->GetMainWeapon();
		if (false == IsValid(mainWeapon))
		{
			return;
		}

		// 레거시 Abort: InAction 이면 End 만, 아니면 Begin 후 End 로 무기 상태를 재동기화.
		if (mainWeapon->InAction)
		{
			WeaponComp->End_DoAction(CEAttackType::Common);
			return;
		}

		WeaponComp->Begin_DoAction(CEAttackType::Common);
		WeaponComp->End_DoAction(CEAttackType::Common);
	}

	static void Act_ApplyAbortMagicCleanup(UCMagicComponent* MagicComp)
	{
		if (false == IsValid(MagicComp))
		{
			return;
		}

		ACMagic* const activeMagic = MagicComp->ActiveMagic.Get();
		if (false == IsValid(activeMagic))
		{
			return;
		}

		if (activeMagic->InAction)
		{
			MagicComp->End_DoAction(CEAttackType::Common);
			return;
		}

		MagicComp->Begin_DoAction(CEAttackType::Common);
		MagicComp->End_DoAction(CEAttackType::Common);
	}
} // namespace

UCBTTask_Act::UCBTTask_Act()
{
	bNotifyTick = true;
	NodeName = TEXT("Action");
}

EBTNodeResult::Type UCBTTask_Act::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const EBTNodeResult::Type resultType = Super::ExecuteTask(OwnerComp, NodeMemory);
	CheckTrueResult(resultType == EBTNodeResult::Type::Failed, resultType);

	AAIController* const aiController = OwnerComp.GetAIOwner();
	CheckNullResult(aiController, EBTNodeResult::Failed);

	UCWeaponComponent* const weaponComp = YJJHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	UCMagicComponent* const magicComp = YJJHelpers::GetComponent<UCMagicComponent>(Owner.Get());

	if ((false == IsValid(weaponComp)) && (false == IsValid(magicComp)))
	{
		CLog::Log(FString::Printf(
			TEXT("[UCBTTask_Act] Weapon/Magic 컴포넌트 없음 — Owner=%s"),
			*GetNameSafe(Owner.Get())));
		return EBTNodeResult::Failed;
	}

	aiController->StopMovement();
	Act_DispatchDoAction(weaponComp, magicComp);

	return EBTNodeResult::InProgress;
}

void UCBTTask_Act::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	(void)NodeMemory;
	(void)DeltaSeconds;

	UCStateComponent* const stateComp = StateComp.Get();
	if (false == IsValid(stateComp))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UCWeaponComponent* const weaponComp = YJJHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	UCMagicComponent* const magicComp = YJJHelpers::GetComponent<UCMagicComponent>(Owner.Get());

	const bool weaponTerminal = Act_IsWeaponActionTerminal(weaponComp, stateComp);
	const bool magicTerminal = Act_IsMagicActionTerminal(magicComp, stateComp);

	if (weaponTerminal || magicTerminal)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UCBTTask_Act::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UCWeaponComponent* const weaponComp = YJJHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	UCMagicComponent* const magicComp = YJJHelpers::GetComponent<UCMagicComponent>(Owner.Get());

	Act_ApplyAbortWeaponCleanup(weaponComp);
	Act_ApplyAbortMagicCleanup(magicComp);

	return Super::AbortTask(OwnerComp, NodeMemory);
}
