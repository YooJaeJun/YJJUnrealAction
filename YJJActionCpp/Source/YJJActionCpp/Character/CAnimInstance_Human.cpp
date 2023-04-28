#include "CAnimInstance_Human.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"

void UCAnimInstance_Human::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Owner = Cast<ACCommonCharacter>(TryGetPawnOwner());
	CheckNull(Owner);

	StateComponent = CHelpers::GetComponent<UCStateComponent>(Owner.Get());
	CheckNull(StateComponent);
	StateComponent->OnStateTypeChanged.AddDynamic(this, &UCAnimInstance_Human::OnStateTypeChanged);

	WeaponComponent = CHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	CheckNull(WeaponComponent);
	WeaponComponent->OnWeaponTypeChanged.AddDynamic(this, &UCAnimInstance_Human::OnWeaponTypeChanged);
}

void UCAnimInstance_Human::NativeUpdateAnimation(float DeltaSeconds)
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
	PrevRotation = UKismetMathLibrary::RInterpTo(PrevRotation, delta, DeltaSeconds, 25);
	Direction = PrevRotation.Yaw;
	Pitch = UKismetMathLibrary::FInterpTo(Pitch, Owner->GetBaseAimRotation().Pitch, DeltaSeconds, 25);

	Falling = (StateType == EStateType::Fall);
	Hitting = (StateType == EStateType::Hit);


}

void UCAnimInstance_Human::OnWeaponTypeChanged(EWeaponType InPrevType, EWeaponType InNewType)
{
	WeaponType = InNewType;
	WeaponPrevType = InPrevType;
}

void UCAnimInstance_Human::OnStateTypeChanged(EStateType InPrevType, EStateType InNewType)
{
	StateType = InNewType;
	StatePrevType = InPrevType;
}
