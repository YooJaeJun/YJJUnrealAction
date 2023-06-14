#include "Components/CFlyComponent.h"
#include "Global.h"
#include "Components/CapsuleComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"

UCFlyComponent::UCFlyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCFlyComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCFlyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCFlyComponent::InputAxis_MoveForward(const float InAxis)
{
	CheckNull(Owner->MovementComp);
	CheckFalse(Owner->MovementComp->CanMove());

	const FRotator rotator = FRotator(0, Owner->GetControlRotation().Yaw, 0);
	const FVector direction = FQuat(rotator).GetForwardVector();

	Owner->AddMovementInput(direction, InAxis);

	CheckFalse(IsFlying());

	const FVector targetLocation = 
		Owner->GetActorLocation() +
		Owner->GetCapsuleComponent()->GetForwardVector() * InAxis * MoveingFactor * GetWorld()->DeltaTimeSeconds;

	Owner->SetActorLocation(targetLocation);
}

void UCFlyComponent::InputAxis_MoveRight(const float InAxis)
{
	CheckNull(Owner->MovementComp);

	if (Owner->MovementComp->CanMove())
	{
		const FRotator rotator = FRotator(0, Owner->GetControlRotation().Yaw, 0);
		const FVector direction = FQuat(rotator).GetRightVector();

		Owner->AddMovementInput(direction, InAxis);

		CheckFalse(IsFlying());

		FTransform targetTransform;

		targetTransform.SetLocation(
			Owner->GetActorLocation() +
			Owner->GetCapsuleComponent()->GetForwardVector() * InAxis * MoveingFactor * GetWorld()->DeltaTimeSeconds
		);

		const FRotator tempTargetRotator(
			Owner->GetActorRotation().Pitch,
			Owner->GetActorRotation().Yaw,
			UKismetMathLibrary::Clamp(Owner->GetActorRotation().Roll + InAxis, -60.0f, 60.0f)
		);

		targetTransform.SetRotation(FQuat(tempTargetRotator));

		Owner->SetActorTransform(targetTransform);
	}//if(Owner->MovementComp->CanMove())
	else
	{
		CheckFalse(IsFlying());

		const FRotator current = Owner->GetActorRotation();
		const FRotator target(Owner->GetActorRotation().Pitch, Owner->GetActorRotation().Yaw, 0);

		Owner->SetActorRotation(
			UKismetMathLibrary::RInterpTo(current, target, GetWorld()->DeltaTimeSeconds, InterpSpeed)
		);
	}
}

void UCFlyComponent::InputAxis_HorizontalLook(const float InAxis)
{
	CheckNull(Owner->MovementComp);
	CheckTrue(Owner->MovementComp->GetFixedCamera());

	Owner->AddControllerYawInput(InAxis * HorizontalLook * GetWorld()->GetDeltaSeconds());
}

void UCFlyComponent::InputAxis_VerticalLook(const float InAxis)
{
	CheckNull(Owner->MovementComp);
	CheckTrue(Owner->MovementComp->GetFixedCamera());

	Owner->AddControllerPitchInput(InAxis * VerticalLook * GetWorld()->GetDeltaSeconds());
}

void UCFlyComponent::InputAxis_FlyUp(const float InAxis)
{
}

void UCFlyComponent::InputAction_Jump()
{
	CheckNull(Owner->StateComp);

	if (IsFlying())
	{
		CLog::Log("IsFlying");
	}
	else
	{
		CheckNull(Owner->MovementComp);
		CheckFalse(Owner->MovementComp->CanMove());

		Owner->Jump();
		Owner->StateComp->SetFallMode();
		Owner->MovementComp->SetGravity(0.0f);

		FTransform landEffectTransform = Owner->GetActorTransform();
		landEffectTransform.SetScale3D(landEffectTransform.GetScale3D() *Owner->LandEffectScaleFactor);

		CHelpers::PlayEffect(GetWorld(), Owner->LandEffect, landEffectTransform);
	}
}

bool UCFlyComponent::IsFlying() const
{
	CheckNullResult(Owner, false);
	CheckNullResult(Owner->StateComp, false);

	return Owner->StateComp->IsFallMode();
}
