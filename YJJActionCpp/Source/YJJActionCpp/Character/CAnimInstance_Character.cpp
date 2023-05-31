#include "Character/CAnimInstance_Character.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"

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
	
	// 라이딩 시 GetPawnOwner가 변경될 경우 대비
	if (nullptr == Owner)
	{
		Owner = Cast<ACCommonCharacter>(TryGetPawnOwner());
		CheckNull(Owner);
	}

	Speed = Owner->GetVelocity().Size2D();

	const FRotator rotator = Owner->GetVelocity().ToOrientationRotator();
	const FRotator rotator2 = Owner->GetControlRotation();
	const FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(rotator, rotator2);
	PrevRotation = UKismetMathLibrary::RInterpTo(
		PrevRotation, delta, DeltaSeconds, 25);
	Direction = PrevRotation.Yaw;
	Pitch = UKismetMathLibrary::FInterpTo(
		Pitch, Owner->GetBaseAimRotation().Pitch, DeltaSeconds, 25);

	if (!!StateComp.Get())
	{
		StateType = StateComp->GetCurMode();

		Falling = (StateType == EStateType::Fall);
		Hitting = (StateType == EStateType::Hit);
	}
}

void UCAnimInstance_Character::OnStateTypeChanged(const EStateType InPrevType, const EStateType InNewType)
{
	StateType = InNewType;
	StatePrevType = InPrevType;
}