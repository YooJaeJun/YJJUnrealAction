#include "Component/CTargetingComponent.h"
#include "Global.h"
#include "Player/CPlayableCharacter.h"

UCTargetingComponent::UCTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCTargetingComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ACPlayableCharacter>(GetOwner());
	CheckNull(Owner);
}

void UCTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCTargetingComponent::Target()
{
	if (IsTargeting)
	{
		
	}
}

void UCTargetingComponent::ChangeFocus(const bool InRight)
{

}

