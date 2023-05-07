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

void UCStateComponent::SetFallMode()
{
	ChangeType(EStateType::Fall);
}

void UCStateComponent::SetAvoidMode()
{
	ChangeType(EStateType::Avoid);
}

void UCStateComponent::ChangeType(const EStateType InType)
{
	PrevType = Type;
	Type = InType;

	if (OnStateTypeChanged.IsBound())
	{
		CLog::Print(Owner->GetName() + " : " + CHelpers::GetEnumToString(Type), -1, 5, FColor::Emerald);
		OnStateTypeChanged.Broadcast(PrevType, Type);
	}
}