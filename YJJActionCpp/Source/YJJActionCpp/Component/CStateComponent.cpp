#include "Component/CStateComponent.h"
#include "Global.h"

void UCStateComponent::SetIdleMode()
{
	Type = EStateType::Idle;
}

void UCStateComponent::SetEquipMode()
{
	Type = EStateType::Equip;
}

void UCStateComponent::SetActMode()
{
	Type = EStateType::Act;
}

void UCStateComponent::SetHitMode()
{
	Type = EStateType::Hit;
}

void UCStateComponent::SetFallMode()
{
	Type = EStateType::Fall;
}

void UCStateComponent::ChangedType(EStateType InType)
{
	EStateType prevType = Type;
	Type = InType;

	if (OnStateTypeChanged.IsBound())
		OnStateTypeChanged.Broadcast(prevType, Type);
}

UCStateComponent::UCStateComponent()
{
}

void UCStateComponent::BeginPlay()
{
	Super::BeginPlay();
}