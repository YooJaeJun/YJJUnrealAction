#include "Components/CTargetingComponent.h"
#include "Global.h"
#include "Player/CPlayableCharacter.h"

UCTargetingComponent::UCTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACPlayableCharacter>(GetOwner());
}

void UCTargetingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCTargetingComponent::InputAction_Targeting()
{
	if (IsTargeting)
	{

	}
}

void UCTargetingComponent::ChangeFocus(const bool InRight)
{

}
