#include "Characters/CAnimInstance_Human.h"

#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCAnimInstance_Human::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ResolveMovementComponent();
}

void UCAnimInstance_Human::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	// Possess 순서에 따라 Initialize 시에는 폰이 비어 있는 경우가 있어 BeginPlay 에서 한 번 더 잡는다.
	ResolveMovementComponent();
}

void UCAnimInstance_Human::ResolveMovementComponent()
{
	MovementComponent = nullptr;

	APawn* const pawnOwner = TryGetPawnOwner();
	if (nullptr == pawnOwner)
	{
		return;
	}

	ACharacter* const pawnChar = Cast<ACharacter>(pawnOwner);
	if (nullptr == pawnChar)
	{
		return;
	}

	MovementComponent = pawnChar->GetCharacterMovement();
}

void UCAnimInstance_Human::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (false == IsValid(OwningCharacter))
	{
		return;
	}

	if (false == IsValid(MovementComponent))
	{
		ResolveMovementComponent();
	}

	if (false == IsValid(MovementComponent))
	{
		return;
	}

	Velocity = MovementComponent->Velocity;
	GroundSpeed = static_cast<double>(Velocity.Size2D());

	const FVector accel = MovementComponent->GetCurrentAcceleration();
	const bool bAccelNonZero = false == accel.IsNearlyZero(KINDA_SMALL_NUMBER);
	const bool bSpeedAboveEpsilon = GroundSpeed > ShouldMoveGroundSpeedEpsilon;

	ShouldMove = bSpeedAboveEpsilon && bAccelNonZero;
	IsFalling = MovementComponent->IsFalling();

	(void)DeltaSeconds;
}
