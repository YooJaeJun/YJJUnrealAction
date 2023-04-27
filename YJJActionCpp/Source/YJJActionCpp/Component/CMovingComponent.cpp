#include "Component/CMovingComponent.h"
#include "Global.h"
#include "Character/CCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UCMovingComponent::UCMovingComponent()
{
}

void UCMovingComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ACCharacter>(GetOwner());
}

void UCMovingComponent::EnableControlRotation()
{
}

void UCMovingComponent::DisableControlRotation()
{
}

void UCMovingComponent::SetSpeeds(const TArray<float> InSpeeds)
{
	for (int32 i = 0; i < InSpeeds.Num(); i++)
		Speeds[i] = InSpeeds[i];
}

void UCMovingComponent::SetMaxWalkSpeed(const ESpeedType Index)
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[(uint8)Index];
}

void UCMovingComponent::SetLerpMove()
{
}

void UCMovingComponent::IsLerpMove()
{
}

void UCMovingComponent::SetGravity()
{
}

void UCMovingComponent::AddGravity()
{
}
