#include "Components/CMovementComponent.h"
#include "Components/CStateComponent.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UCMovementComponent::UCMovementComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());

	if (Owner.IsValid())
		StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());
}

void UCMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCMovementComponent::OnEnableTopViewCam()
{
	bTopViewCam = true;
}

void UCMovementComponent::OffEnableTopViewCam()
{
	bTopViewCam = false;
}

void UCMovementComponent::SetSpeeds(const TArray<float> InSpeeds)
{
	for (int32 i = 0; i < InSpeeds.Num(); i++)
		Speeds[i] = InSpeeds[i];
}

void UCMovementComponent::SetSpeed(const CESpeedType Index) const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[static_cast<uint8>(Index)];
}

void UCMovementComponent::SetMaxWalkSpeed(const float Speed) const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void UCMovementComponent::SetWalkSpeed() const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[static_cast<uint8>(CESpeedType::Walk)];
}

void UCMovementComponent::SetRunSpeed() const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[static_cast<uint8>(CESpeedType::Run)];
}

void UCMovementComponent::SetSprintSpeed() const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[static_cast<uint8>(CESpeedType::Sprint)];
}

void UCMovementComponent::SetGravity(const float InValue) const
{
	Owner->GetCharacterMovement()->GravityScale = InValue;
}

void UCMovementComponent::AddGravity(const float InValue) const
{
	SetGravity(Owner->GetCharacterMovement()->GravityScale + InValue);
}

void UCMovementComponent::SetJumpZ(const float InVelocity) const
{
	Owner->GetCharacterMovement()->JumpZVelocity = InVelocity;
}

void UCMovementComponent::SetFriction(const float InFriction, const float InBraking) const
{
	Owner->GetCharacterMovement()->GroundFriction = InFriction;
	Owner->GetCharacterMovement()->BrakingDecelerationWalking = InBraking;
}

void UCMovementComponent::InputAxis_MoveForward(const float InAxis)
{
	CheckFalse(CanMove(InAxis));
	CheckTrue(StateComp->IsFallMode());

	Forward = InAxis;

	const FRotator rotator = FRotator(0, Owner->GetControlRotation().Yaw, 0);
	FVector direction = FQuat(rotator).GetForwardVector();

	if (true == bTopViewCam)
		direction = FVector::XAxisVector;

	Owner->AddMovementInput(direction, InAxis);
}

void UCMovementComponent::InputAxis_MoveRight(const float InAxis)
{
	CheckFalse(CanMove(InAxis));
	CheckTrue(StateComp->IsFallMode());

	Right = InAxis;

	const FRotator rotator = FRotator(0, Owner->GetControlRotation().Yaw, 0);
	FVector direction = FQuat(rotator).GetRightVector();

	if (true == bTopViewCam)
		direction = FVector::YAxisVector;

	Owner->AddMovementInput(direction, InAxis);
}

void UCMovementComponent::InputAction_Walk()
{
	SetMaxWalkSpeed(Speeds[static_cast<uint8>(CESpeedType::Walk)]);

	SpeedFactor = 1.0f;
}

void UCMovementComponent::InputAction_Run()
{
	SetMaxWalkSpeed(Speeds[static_cast<uint8>(CESpeedType::Sprint)]);

	SpeedFactor = 4.0f;
}

void UCMovementComponent::InputAction_Jump()
{
	CheckFalse(CanMove());

	Owner->Jump();
	StateComp->SetFallMode();
}

bool UCMovementComponent::CanMove(const float InAxis) const
{
	if (abs(InAxis) > 0.5f)
		return CanMove();

	return false;
}
