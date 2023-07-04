#include "Characters/CAnimInstance_Character.h"
#include "Global.h"
#include "Animals/CAnimal_AI.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CMovementComponent.h"
#include "Components/CFlyComponent.h"
#include "Characters/Animals/Dragon/CDragon_AI.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCAnimInstance_Character::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Owner = Cast<ACCommonCharacter>(TryGetPawnOwner());
	CheckNull(Owner);

	StateComp = CHelpers::GetComponent<UCStateComponent>(Owner.Get());

	if (!!StateComp.Get())
		StateComp->OnStateTypeChanged.AddUniqueDynamic(this, &UCAnimInstance_Character::OnStateTypeChanged);

	MovementComp = CHelpers::GetComponent<UCMovementComponent>(Owner.Get());
	FlyComp = CHelpers::GetComponent<UCFlyComponent>(Owner.Get());
}

void UCAnimInstance_Character::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	// ���̵� ž�� ���� TryGetPawnOwner�� ����� ��� ���
	if (nullptr == Owner)
	{
		Owner = Cast<ACCommonCharacter>(TryGetPawnOwner());
		CheckNull(Owner);
	}

	if (!!StateComp.Get())
	{
		StateType = StateComp->GetCurMode();

		bFalling = (StateComp->IsFallMode());
		bHitting = (StateComp->IsHitMode());
	}

	// Move

	//Speed = Owner->GetVelocity().Size2D();

	//const FRotator rotator = UKismetMathLibrary::MakeRotFromX(Owner->GetVelocity());
	//const FRotator rotator2 = Owner->GetControlRotation();
	//const FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(rotator, rotator2);

	//PrevRotation = UKismetMathLibrary::RInterpTo(delta, PrevRotation, DeltaSeconds, 25);

	//Direction = PrevRotation.Yaw;

	//Pitch = UKismetMathLibrary::FInterpTo(
	//	Pitch, Owner->GetBaseAimRotation().Pitch, DeltaSeconds, 25);

	Speed = Owner->GetVelocity().Size();

	const FRotator currentRot = UKismetMathLibrary::MakeRotator(Pitch, Yaw, 0.0f);

	const FRotator targetRot = UKismetMathLibrary::NormalizedDeltaRotator(
		Owner->GetControlRotation(),
		Owner->GetActorRotation());

	PrevRotation = UKismetMathLibrary::RInterpTo(currentRot, targetRot, DeltaSeconds, 25);

	Pitch = UKismetMathLibrary::Clamp(PrevRotation.Pitch, -90.0f, 90.0f);
	Yaw = UKismetMathLibrary::Clamp(PrevRotation.Yaw, -90.0f, 90.0f);

	if (Owner->GetVelocity().Size() <= 0.0f)
		Direction = 0.0f;
	else
	{
		const FRotator currentRotator = UKismetMathLibrary::NormalizedDeltaRotator(
			UKismetMathLibrary::MakeRotFromX(Owner->GetVelocity()),
			Owner->GetActorRotation());

		if (currentRotator.Yaw >= 180.0f)
			Direction -= 360.0f;
		else
			Direction = currentRotator.Yaw;
	}//Owner->GetVelocity().Size() > 0.0f

	// Animation

	if (!!MovementComp.Get())
	{
		const FRotator rotator(0, Owner->GetControlRotation().Yaw, 0);

		FVector forward = FVector::ZeroVector;
		FVector right = FVector::ZeroVector;

		if (!!FlyComp.Get())
		{
			const TWeakObjectPtr<ACDragon_AI> FlyingCharacter = Cast<ACDragon_AI>(Owner);

			CheckNull(FlyingCharacter);

			forward = UKismetMathLibrary::GetForwardVector(rotator)
				* FlyingCharacter->FlyComp->Forward;

			right = UKismetMathLibrary::GetRightVector(rotator)
				* FlyingCharacter->FlyComp->Right;
		}//!!FlyComp
		else
		{
			forward = UKismetMathLibrary::GetForwardVector(rotator)
				* MovementComp->Forward;

			right = UKismetMathLibrary::GetRightVector(rotator)
				* MovementComp->Right;
		}

		const FVector current = (forward + right) * MovementComp->SpeedFactor;

		const float dotForward = UKismetMathLibrary::Dot_VectorVector(current, Owner->GetActorForwardVector());
		const float dotRight = UKismetMathLibrary::Dot_VectorVector(current, Owner->GetActorRightVector());

		Forward = UKismetMathLibrary::Lerp(Forward, dotForward, 0.05f);
		Side = UKismetMathLibrary::Lerp(Side, dotRight, 0.05f);
	}//MovementComp
}

void UCAnimInstance_Character::OnStateTypeChanged(const EStateType InPrevType, const EStateType InNewType)
{
	StateType = InNewType;
	StatePrevType = InPrevType;
}