#include "Components/CStateComponent.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"

UCStateComponent::UCStateComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCStateComponent::BeginPlay()
{
	Super::BeginPlay();
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

void UCStateComponent::SetHitMode()
{
	ChangeType(EStateType::Hit);
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
	PrevType = Type;
	Type = InType;

	if (OnStateTypeChanged.IsBound())
	{
		CLog::Print(Owner->GetName() + " : " + CHelpers::ConvertEnumToString(Type), -1, 5, FColor::Emerald);
		OnStateTypeChanged.Broadcast(PrevType, Type);
	}
}