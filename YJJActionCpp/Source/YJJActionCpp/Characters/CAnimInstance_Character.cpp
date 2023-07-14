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

	StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());

	if (StateComp.IsValid())
		StateComp->OnStateTypeChanged.AddUniqueDynamic(this, &UCAnimInstance_Character::OnStateTypeChanged);

	MovementComp = YJJHelpers::GetComponent<UCMovementComponent>(Owner.Get());
	FlyComp = YJJHelpers::GetComponent<UCFlyComponent>(Owner.Get());
}

void UCAnimInstance_Character::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	// 라이딩 탑승 시점 TryGetPawnOwner가 변경될 경우 대비
	if (nullptr == Owner)
	{
		Owner = Cast<ACCommonCharacter>(TryGetPawnOwner());
		CheckNull(Owner);
	}

	if (StateComp.IsValid())
	{
		StateType = StateComp->GetCurMode();

		bFalling = (StateComp->IsFallMode());
		bHitting = (false == StateComp->IsHitNoneMode());
	}

	// Move

	Speed = Owner->GetVelocity().Size();

	const FRotator rotator = UKismetMathLibrary::MakeRotFromX(Owner->GetVelocity());
	const FRotator rotator2 = Owner->GetControlRotation();
	const FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(rotator, rotator2);

	PrevRotation = UKismetMathLibrary::RInterpTo(delta, PrevRotation, DeltaSeconds, 25);

	Direction = PrevRotation.Yaw;

	Pitch = UKismetMathLibrary::FInterpTo(Pitch, Owner->GetBaseAimRotation().Pitch, DeltaSeconds, 25);
	Yaw = UKismetMathLibrary::FInterpTo(Yaw, Owner->GetBaseAimRotation().Yaw, DeltaSeconds, 25);

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

	CheckNull(MovementComp);

	FVector forward = FVector::ZeroVector;
	FVector right = FVector::ZeroVector;

	if (FlyComp.IsValid())
	{
		const TWeakObjectPtr<ACDragon_AI> FlyingCharacter = Cast<ACDragon_AI>(Owner);

		CheckNull(FlyingCharacter);

		forward = UKismetMathLibrary::GetForwardVector(rotator)
			* FlyingCharacter->FlyComp->Forward;

		right = UKismetMathLibrary::GetRightVector(rotator)
			* FlyingCharacter->FlyComp->Right;
	}//FlyComp.IsValid()
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
}

void UCAnimInstance_Character::OnStateTypeChanged(const CEStateType InPrevType, const CEStateType InNewType)
{
	StateType = InNewType;
	StatePrevType = InPrevType;
}