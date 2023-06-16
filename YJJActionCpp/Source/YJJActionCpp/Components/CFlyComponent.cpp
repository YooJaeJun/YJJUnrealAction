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
	//CheckFalse(Owner->MovementComp->CanMove(InAxis));

	if (Owner->MovementComp->CanMove(InAxis))
	{
		const FRotator rotator = FRotator(0, Owner->GetControlRotation().Yaw, 0);
		const FVector direction = FQuat(rotator).GetForwardVector();

		Owner->AddMovementInput(direction, InAxis);

		CheckFalse(IsFlying());

		const FVector targetLocation =
			Owner->GetActorLocation() +
			Owner->GetCapsuleComponent()->GetForwardVector() * InAxis * MovingFactor * GetWorld()->DeltaTimeSeconds;

		Owner->SetActorLocation(targetLocation);
	}
}

void UCFlyComponent::InputAxis_MoveRight(const float InAxis)
{
	CheckNull(Owner->MovementComp);
	//CheckFalse(Owner->MovementComp->CanMove(InAxis));

	Right = InAxis;

	if (Owner->MovementComp->CanMove(InAxis))
	{
		const FRotator rotator = FRotator(0, Owner->GetControlRotation().Yaw, 0);
		const FVector direction = FQuat(rotator).GetRightVector();

		Owner->AddMovementInput(direction, InAxis);

		CheckFalse(IsFlying());

		FTransform targetTransform;

		targetTransform.SetLocation(
			Owner->GetActorLocation() +
			Owner->GetCapsuleComponent()->GetForwardVector() * InAxis * MovingFactor * GetWorld()->DeltaTimeSeconds);

		const FRotator targetRotator(
			Owner->GetActorRotation().Pitch,
			Owner->GetActorRotation().Yaw,
			UKismetMathLibrary::Clamp(Owner->GetActorRotation().Roll + InAxis, -60.0f, 60.0f));

		targetTransform.SetRotation(FQuat(targetRotator));

		Owner->SetActorTransform(targetTransform);
	}//if(Owner->MovementComp->CanMove())
	else
	{
		CheckFalse(IsFlying());

		const FRotator current = Owner->GetActorRotation();
		const FRotator target(Owner->GetActorRotation().Pitch, Owner->GetActorRotation().Yaw, 0);

		Owner->SetActorRotation(
			UKismetMathLibrary::RInterpTo(current, target, GetWorld()->DeltaTimeSeconds, InterpSpeed));
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
	CheckNull(Owner->MovementComp);
	CheckFalse(Owner->MovementComp->CanMove(InAxis));

	if (!!Owner->MovementComp->CanMove())
	{
		UpFactor = (InAxis > 0.0f) ? 20.0f : -50.0f;

		const FVector targetLocation = Owner->GetActorLocation() +
			(Owner->GetCapsuleComponent()->GetForwardVector() * InAxis * UpFactor);

		const float pitch = UKismetMathLibrary::Clamp(
			Owner->GetActorRotation().Pitch + InAxis,
			-60.0f,
			60.0f);

		const FRotator targetRotation = UKismetMathLibrary::MakeRotator(
			pitch,
			Owner->GetActorRotation().Yaw,
			Owner->GetActorRotation().Roll);

		const FTransform targetTransform(
			FQuat(targetRotation),
			targetLocation,
			FVector::OneVector);

		Owner->SetActorTransform(targetTransform);
	}//!!Owner->MovementComp->CanMove()
	else if (!!IsFlying())
	{
		const FRotator currentRotation = Owner->GetActorRotation();

		const FRotator targetRotation(0.0f,
			Owner->GetActorRotation().Yaw,
			Owner->GetActorRotation().Roll);

		Owner->SetActorRotation(UKismetMathLibrary::RInterpTo(
			currentRotation,
			targetRotation,
			UGameplayStatics::GetWorldDeltaSeconds(GetWorld()),
			InterpSpeed));
	}
}

void UCFlyComponent::InputAction_Jump()
{
	CheckNull(Owner->StateComp);

	if (IsFlying())
		LandOn();
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

void UCFlyComponent::LandOn() const
{
	CheckNull(Owner->StateComp);
	CheckNull(Owner->MovementComp);

	const FVector start = Owner->GetActorLocation();

	const FVector endZFactor = Owner->GetMesh()->GetUpVector() * -6000.0f;

	const float endZ = 
		Owner->GetActorLocation().Z +
		endZFactor.Z;

	const FVector end(Owner->GetActorLocation().X,
		Owner->GetActorLocation().Y,
		endZ);

	const TArray<AActor*> ignores;

	FHitResult hitResult;

	bool bHit = true;

	bHit &= UKismetSystemLibrary::LineTraceSingleByProfile(
		GetWorld(),
		start, end,
		"IgnoreOnlyPawn",
		false,
		ignores,
		EDrawDebugTrace::None,
		hitResult,
		true);

	CheckFalse(bHit);

	Owner->StateComp->SetFallMode();
	Owner->MovementComp->SetGravity(1.0f);

	const FVector targetLocation =
		hitResult.ImpactPoint + 
		Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const FRotator targetRotation = Owner->GetActorRotation();

	FLatentActionInfo latentInfo;
	latentInfo.CallbackTarget = Owner.Get();

	UKismetSystemLibrary::MoveComponentTo(
		Owner->GetRootComponent(),
		targetLocation,
		targetRotation,
		true, true,
		1.0f, false,
		EMoveComponentAction::Move,
		latentInfo);
}
