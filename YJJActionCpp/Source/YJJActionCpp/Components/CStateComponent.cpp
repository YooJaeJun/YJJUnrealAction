#include "Components/CStateComponent.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"

UCStateComponent::UCStateComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCStateComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UCStateComponent::CanAttack() const
{
	return IsIdleMode();
}

void UCStateComponent::SetIdleMode()
{
	ChangeType(EStateType::Idle);
}

void UCStateComponent::SetFallMode()
{
	ChangeType(EStateType::Fall);
}

void UCStateComponent::SetAvoidMode()
{
	ChangeType(EStateType::Avoid);
}

void UCStateComponent::SetEquipMode()
{
	ChangeType(EStateType::Equip);
}

void UCStateComponent::SetActMode()
{
	ChangeType(EStateType::Act);
}

void UCStateComponent::SetDeadMode()
{
	ChangeType(EStateType::Dead);
}

void UCStateComponent::SetRiseMode()
{
	ChangeType(EStateType::Rise);
}

void UCStateComponent::ChangeType(const EStateType InType)
{
	PrevType = CurType;
	CurType = InType;

	if (OnStateTypeChanged.IsBound())
	{
		CLog::Print(Owner->GetName() + " : " 
			+ YJJHelpers::ConvertEnumToString(CurType), 
			-1, 
			5, 
			FColor::Emerald);

		OnStateTypeChanged.Broadcast(PrevType, CurType);
	}
}

void UCStateComponent::SetHitMode(const EHitType InHitType)
{
	ChangeHitType(InHitType);
}

void UCStateComponent::SetHitNoneMode()
{
	ChangeHitType(EHitType::None);
}

void UCStateComponent::SetHitCommonMode()
{
	ChangeHitType(EHitType::Common);
}

void UCStateComponent::SetHitDownMode()
{
	ChangeHitType(EHitType::Down);
}

void UCStateComponent::SetHitFlyMode()
{
	ChangeHitType(EHitType::Fly);
}

void UCStateComponent::SetHitKnockbackMode()
{
	ChangeHitType(EHitType::Knockback);
}

void UCStateComponent::SetHitAirMode()
{
	ChangeHitType(EHitType::Air);
}

void UCStateComponent::SetHitFlyingPutDownMode()
{
	ChangeHitType(EHitType::FlyingPutDown);
}

void UCStateComponent::ChangeHitType(const EHitType InType)
{
	PrevHitType = CurHitType;
	CurHitType = InType;

	if (OnHitStateTypeChanged.IsBound())
	{
		CLog::Print(Owner->GetName() + " : "
			+ YJJHelpers::ConvertEnumToString(CurHitType),
			-1,
			5,
			FColor::Orange);

		OnHitStateTypeChanged.Broadcast(PrevHitType, CurHitType);
	}
}