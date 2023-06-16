#include "Components/CMovementComponent.h"
#include "Components/CStateComponent.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UCMovementComponent::UCMovementComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCMovementComponent::EnableControlRotation() const
{
	Owner->bUseControllerRotationYaw = true;
	Owner->GetCharacterMovement()->bOrientRotationToMovement = false;
}

void UCMovementComponent::DisableControlRotation() const
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

void UCMovementComponent::SetWalkSpeed() const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[(uint8)ESpeedType::Walk];
}

void UCMovementComponent::SetRunSpeed() const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[(uint8)ESpeedType::Run];
}

void UCMovementComponent::SetSprintSpeed() const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[(uint8)ESpeedType::Sprint];
}

void UCMovementComponent::SetLerpMove() const
{
}

void UCMovementComponent::IsLerpMove() const
{
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
	//CheckFalse(CanMove(InAxis));
	CheckTrue(Owner->StateComp->IsFallMode());

	Forward = InAxis;

	const FRotator rotator = FRotator(0, Owner->GetControlRotation().Yaw, 0);
	const FVector direction = FQuat(rotator).GetForwardVector();

	Owner->AddMovementInput(direction, InAxis);
}

void UCMovementComponent::InputAxis_MoveRight(const float InAxis)
{
	//CheckFalse(CanMove(InAxis));
	CheckTrue(Owner->StateComp->IsFallMode());

	Right = InAxis;

	const FRotator rotator = FRotator(0, Owner->GetControlRotation().Yaw, 0);
	const FVector direction = FQuat(rotator).GetRightVector();

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

	SpeedFactor = 1.0f;
}

void UCMovementComponent::InputAction_Run()
{
	SetMaxWalkSpeed(Speeds[static_cast<uint8>(ESpeedType::Sprint)]);

	SpeedFactor = 4.0f;
}

void UCMovementComponent::InputAction_Jump()
{
	CheckFalse(CanMove());

	Owner->Jump();
	Owner->StateComp->SetFallMode();
}

bool UCMovementComponent::CanMove(const float InAxis) const
{
	if (abs(InAxis) > 0.5f)
		return CanMove();

	return false;
}
