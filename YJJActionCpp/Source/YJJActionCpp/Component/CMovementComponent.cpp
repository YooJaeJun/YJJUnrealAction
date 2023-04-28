#include "Component/CMovementComponent.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UCMovementComponent::UCMovementComponent()
{
}

void UCMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCMovementComponent::EnableControlRotation()
{
}

void UCMovementComponent::DisableControlRotation()
{
}

void UCMovementComponent::SetSpeeds(const TArray<float> InSpeeds)
{
	for (int32 i = 0; i < InSpeeds.Num(); i++)
		Speeds[i] = InSpeeds[i];
}

void UCMovementComponent::SetSpeed(const ESpeedType Index) const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[(uint8)Index];
}

void UCMovementComponent::SetMaxWalkSpeed(const float Speed) const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void UCMovementComponent::SetWalkSpeed(const float InSpeed) const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[(uint8)ESpeedType::Walk];
}

void UCMovementComponent::SetRunSpeed(const float InSpeed) const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[(uint8)ESpeedType::Run];
}

void UCMovementComponent::SetSprintSpeed(const float InSpeed) const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[(uint8)ESpeedType::Sprint];
}

void UCMovementComponent::SetLerpMove()
{
}

void UCMovementComponent::IsLerpMove() const
{
}

void UCMovementComponent::SetGravity()
{
}

void UCMovementComponent::AddGravity()
{
}
