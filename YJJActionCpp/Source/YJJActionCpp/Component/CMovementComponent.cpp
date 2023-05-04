#include "Component/CMovementComponent.h"

#include "CStateComponent.h"
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
	Owner->bUseControllerRotationYaw = true;
	Owner->GetCharacterMovement()->bOrientRotationToMovement = false;
}

void UCMovementComponent::DisableControlRotation()
{
	Owner->bUseControllerRotationYaw = false;
	Owner->GetCharacterMovement()->bOrientRotationToMovement = true;
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

void UCMovementComponent::InputAxis_MoveForward(const float InAxis)
{
	CheckFalse(bCanMove);

	FRotator rotator = FRotator(0, Owner->GetControlRotation().Yaw, 0);
	FVector direction = FQuat(rotator).GetForwardVector();

	Owner->AddMovementInput(direction, InAxis);
}

void UCMovementComponent::InputAxis_MoveRight(const float InAxis)
{
	CheckFalse(bCanMove);

	FRotator rotator = FRotator(0, Owner->GetControlRotation().Yaw, 0);
	FVector direction = FQuat(rotator).GetRightVector();

	Owner->AddMovementInput(direction, InAxis);
}

void UCMovementComponent::InputAxis_HorizontalLook(const float InAxis)
{
	CheckTrue(bFixedCamera);

	Owner->AddControllerYawInput(InAxis * HorizontalLook * GetWorld()->GetDeltaSeconds());
}

void UCMovementComponent::InputAxis_VerticalLook(const float InAxis)
{
	CheckTrue(bFixedCamera);

	Owner->AddControllerPitchInput(InAxis * VerticalLook * GetWorld()->GetDeltaSeconds());
}

void UCMovementComponent::InputAction_Walk()
{
	SetMaxWalkSpeed(Speeds[static_cast<uint8>(ESpeedType::Walk)]);
}

void UCMovementComponent::InputAction_Run()
{
	SetMaxWalkSpeed(Speeds[static_cast<uint8>(ESpeedType::Sprint)]);
}

void UCMovementComponent::InputAction_Jump()
{
	CheckFalse(CanMove());

	Owner->Jump();

	Owner->StateComponent->SetFallMode();
}