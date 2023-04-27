#include "Component/CStateComponent.h"
#include "Global.h"
#include "Character/CCharacter.h"

void UCStateComponent::SetIdleMode()
{
}

void UCStateComponent::SetEquipMode()
{
}

void UCStateComponent::SetActMode()
{
}

void UCStateComponent::SetHitMode()
{
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