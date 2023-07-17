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
	ChangeType(CEStateType::Idle);
}

void UCStateComponent::SetFallMode()
{
	ChangeType(CEStateType::Fall);
}

void UCStateComponent::SetAvoidMode()
{
	ChangeType(CEStateType::Avoid);
}

void UCStateComponent::SetEquipMode()
{
	ChangeType(CEStateType::Equip);
}

void UCStateComponent::SetActMode()
{
	ChangeType(CEStateType::Act);
}

void UCStateComponent::SetDeadMode()
{
	ChangeType(CEStateType::Dead);
}

void UCStateComponent::SetRiseMode()
{
	ChangeType(CEStateType::Rise);
}

void UCStateComponent::ChangeType(const CEStateType InType)
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

void UCStateComponent::SetHitMode(const CEHitType InHitType)
{
	ChangeHitType(InHitType);
}

void UCStateComponent::SetHitNoneMode()
{
	ChangeHitType(CEHitType::None);
}

void UCStateComponent::SetHitCommonMode()
{
	ChangeHitType(CEHitType::Common);
}

void UCStateComponent::SetHitDownMode()
{
	ChangeHitType(CEHitType::Down);
}

void UCStateComponent::SetHitFlyMode()
{
	ChangeHitType(CEHitType::Fly);
}

void UCStateComponent::SetHitKnockbackMode()
{
	ChangeHitType(CEHitType::Knockback);
}

void UCStateComponent::SetHitAirMode()
{
	ChangeHitType(CEHitType::Air);
}

void UCStateComponent::SetHitFlyingPutDownMode()
{
	ChangeHitType(CEHitType::FlyingPutDown);
}

void UCStateComponent::ChangeHitType(const CEHitType InType)
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