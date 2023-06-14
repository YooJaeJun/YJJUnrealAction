#include "Characters/CAnimInstance_Character.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCAnimInstance_Character::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Owner = Cast<ACCommonCharacter>(TryGetPawnOwner());
	CheckNull(Owner);

	StateComp = CHelpers::GetComponent<UCStateComponent>(Owner.Get());
	CheckNull(StateComp);
	StateComp->OnStateTypeChanged.AddDynamic(this, &UCAnimInstance_Character::OnStateTypeChanged);
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

	if (!!StateComp.Get())
	{
		StateType = StateComp->GetCurMode();

		bFalling = (StateComp->IsFallMode());
		bHitting = (StateComp->IsHitMode());
	}

	//Speed = Owner->GetVelocity().Size2D();

	//const FRotator rotator = UKismetMathLibrary::MakeRotFromX(Owner->GetVelocity());
	//const FRotator rotator2 = Owner->GetControlRotation();
	//const FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(rotator, rotator2);

	//PrevRotation = UKismetMathLibrary::RInterpTo(delta, PrevRotation, DeltaSeconds, 25);

	//Direction = PrevRotation.Yaw;

	//Pitch = UKismetMathLibrary::FInterpTo(
	//	Pitch, Owner->GetBaseAimRotation().Pitch, DeltaSeconds, 25);



	Speed = Owner->GetVelocity().Size2D();

	const FRotator current = UKismetMathLibrary::MakeRotator(Pitch, Yaw, 0.0f);

	const FRotator target = UKismetMathLibrary::NormalizedDeltaRotator(
		Owner->GetControlRotation(), 
		Owner->GetActorRotation());

	PrevRotation = UKismetMathLibrary::RInterpTo(current, target, DeltaSeconds, 25);

	Pitch = UKismetMathLibrary::Clamp(PrevRotation.Pitch, -90.0f, 90.0f);
	Yaw = UKismetMathLibrary::Clamp(PrevRotation.Yaw, -90.0f, 90.0f);


	if (Owner->GetVelocity().Size() <= 0.0f)
		Direction = 0.0f;
	else
	{
		const FRotator rotator = UKismetMathLibrary::NormalizedDeltaRotator(
			UKismetMathLibrary::MakeRotFromX(Owner->GetVelocity()),
			Owner->GetActorRotation());

		if (rotator.Yaw >= 180.0f)
			Direction -= 360.0f;
		else
			Direction = rotator.Yaw;
	}
}

void UCAnimInstance_Character::OnStateTypeChanged(const EStateType InPrevType, const EStateType InNewType)
{
	StateType = InNewType;
	StatePrevType = InPrevType;
}