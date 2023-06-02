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

void UCMovementComponent::SetJumpZ(const float InVelocity)
{
	Owner->GetCharacterMovement()->JumpZVelocity = InVelocity;
}

void UCMovementComponent::SetFriction(const float InFriction, const float InBraking)
{
	Owner->GetCharacterMovement()->GroundFriction = InFriction;
	Owner->GetCharacterMovement()->BrakingDecelerationWalking = InBraking;
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

	Owner->StateComp->SetFallMode();
}
