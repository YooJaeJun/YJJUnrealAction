#include "Characters/AI/CBTTask_Equip.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Components/CMagicComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CWeaponComponent.h"
#include "Global.h"
#include "Utilities/YJJHelpers.h"
#include "Weapons/CMagic.h"
#include "Weapons/CWeapon.h"

namespace
{
	static bool Equip_IsWeaponTerminal(UCWeaponComponent* WeaponComp, UCStateComponent* StateComp)
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

		const bool notEquipping = false == mainWeapon->IsEquipping;
		const bool idle = StateComp->IsIdle();
		return notEquipping && idle;
	}

	static bool Equip_IsMagicTerminal(UCMagicComponent* MagicComp, UCStateComponent* StateComp)
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

		const bool notEquipping = false == activeMagic->IsEquipping;
		const bool idle = StateComp->IsIdle();
		return notEquipping && idle;
	}

	static void Equip_ApplyWeaponType(UCWeaponComponent* WeaponComp, CEWeaponType InWeaponType)
	{
		if (false == IsValid(WeaponComp))
		{
			return;
		}

		switch (InWeaponType)
		{
		case CEWeaponType::Unarmed:
			WeaponComp->SetBlueprintUnarmed_WithSpawnedWeapons();
			break;
		case CEWeaponType::Fist:
			WeaponComp->Blueprint_SetFist_WithSpawnedWeapons();
			break;
		case CEWeaponType::Sword:
			WeaponComp->Blueprint_SetSword_WithSpawnedWeapons();
			break;
		case CEWeaponType::Hammer:
			WeaponComp->Blueprint_SetHammer_WithSpawnedWeapons();
			break;
		case CEWeaponType::Bow:
			WeaponComp->Blueprint_SetBow_WithSpawnedWeapons();
			break;
		case CEWeaponType::Dual:
			WeaponComp->Blueprint_SetDual_WithSpawnedWeapons();
			break;
		default:
			// Shield 등 아직 Set* 포팅 없는 타입은 ChangeType 만 맞춘다.
			WeaponComp->ChangeBlueprintWeaponLanes(InWeaponType, InWeaponType);
			break;
		}
	}

	static void Equip_ApplyMagicType(UCMagicComponent* MagicComp, CEMagicType InMagicType)
	{
		if (false == IsValid(MagicComp))
		{
			return;
		}

		switch (InMagicType)
		{
		case CEMagicType::Unarmed:
			MagicComp->SetUnarmed();
			break;
		case CEMagicType::Warp:
			MagicComp->SetWarp();
			break;
		case CEMagicType::Around:
			MagicComp->SetAround();
			break;
		case CEMagicType::FireBall:
			MagicComp->SetFireBall();
			break;
		case CEMagicType::Bomb:
			MagicComp->SetBomb();
			break;
		case CEMagicType::Yondu:
			MagicComp->SetYondu();
			break;
		default:
			MagicComp->SetMode(InMagicType);
			break;
		}
	}

	static void Equip_ApplyAbortWeaponCleanup(UCWeaponComponent* WeaponComp)
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

		if (mainWeapon->IsEquipping)
		{
			WeaponComp->End_Equip();
			return;
		}

		WeaponComp->Begin_Equip();
		WeaponComp->End_Equip();
	}

	static void Equip_ApplyAbortMagicCleanup(UCMagicComponent* MagicComp)
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

		if (activeMagic->IsEquipping)
		{
			MagicComp->End_Equip();
			return;
		}

		MagicComp->Begin_Equip();
		MagicComp->End_Equip();
	}
} // namespace

UCBTTask_Equip::UCBTTask_Equip()
{
	bNotifyTick = true;
	NodeName = TEXT("Equip");
}

EBTNodeResult::Type UCBTTask_Equip::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const EBTNodeResult::Type resultType = Super::ExecuteTask(OwnerComp, NodeMemory);
	CheckTrueResult(resultType == EBTNodeResult::Type::Failed, resultType);

	AAIController* const aiController = OwnerComp.GetAIOwner();
	CheckNullResult(aiController, EBTNodeResult::Failed);

	UCStateComponent* const stateComp = StateComp.Get();
	CheckNullResult(stateComp, EBTNodeResult::Failed);

	UCWeaponComponent* const weaponComp = YJJHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	UCMagicComponent* const magicComp = YJJHelpers::GetComponent<UCMagicComponent>(Owner.Get());

	if ((false == IsValid(weaponComp)) && (false == IsValid(magicComp)))
	{
		CLog::Log(FString::Printf(
			TEXT("[UCBTTask_Equip] Weapon/Magic 컴포넌트 없음 — Owner=%s"),
			*GetNameSafe(Owner.Get())));
		return EBTNodeResult::Failed;
	}

	bool bNeedsEquip = false;

	if (IsValid(weaponComp))
	{
		CEWeaponType currentWeaponType = CEWeaponType::Unarmed;
		weaponComp->GetMainType(currentWeaponType);

		if (currentWeaponType != WeaponType)
		{
			aiController->StopMovement();
			Equip_ApplyWeaponType(weaponComp, WeaponType);
			bNeedsEquip = true;
		}
	}

	if (IsValid(magicComp))
	{
		CEMagicType currentMagicType = CEMagicType::Unarmed;
		magicComp->GetType(currentMagicType);

		if (currentMagicType != MagicType)
		{
			aiController->StopMovement();
			Equip_ApplyMagicType(magicComp, MagicType);
			bNeedsEquip = true;
		}
	}

	const bool weaponTerminal = Equip_IsWeaponTerminal(weaponComp, stateComp);
	const bool magicTerminal = Equip_IsMagicTerminal(magicComp, stateComp);

	bool bAllRelevantTerminal = true;
	if (IsValid(weaponComp))
	{
		bAllRelevantTerminal = bAllRelevantTerminal && weaponTerminal;
	}
	if (IsValid(magicComp))
	{
		bAllRelevantTerminal = bAllRelevantTerminal && magicTerminal;
	}

	if (bAllRelevantTerminal)
	{
		return EBTNodeResult::Succeeded;
	}

	(void)bNeedsEquip;
	return EBTNodeResult::InProgress;
}

void UCBTTask_Equip::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

	const bool weaponTerminal = Equip_IsWeaponTerminal(weaponComp, stateComp);
	const bool magicTerminal = Equip_IsMagicTerminal(magicComp, stateComp);

	if (weaponTerminal || magicTerminal)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UCBTTask_Equip::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UCWeaponComponent* const weaponComp = YJJHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	UCMagicComponent* const magicComp = YJJHelpers::GetComponent<UCMagicComponent>(Owner.Get());

	Equip_ApplyAbortWeaponCleanup(weaponComp);
	Equip_ApplyAbortMagicCleanup(magicComp);

	return Super::AbortTask(OwnerComp, NodeMemory);
}
