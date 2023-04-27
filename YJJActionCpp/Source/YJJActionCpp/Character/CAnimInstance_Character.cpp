#include "CAnimInstance_Character.h"
#include "Global.h"
#include "Character/CCharacter.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"

void UCAnimInstance_Character::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Owner = Cast<ACCharacter>(TryGetPawnOwner());
	CheckNull(Owner);

	WeaponComponent = CHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	CheckNull(WeaponComponent);
	WeaponComponent->OnWeaponTypeChanged.AddDynamic(this, &UCAnimInstance_Character::OnWeaponTypeChanged);

	StateComponent = CHelpers::GetComponent<UCStateComponent>(Owner.Get());
	CheckNull(StateComponent);
	StateComponent->OnStateTypeChanged.AddDynamic(this, &UCAnimInstance_Character::OnStateTypeChanged);
}

void UCAnimInstance_Character::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 라이딩 시 GetPawnOwner가 변경될 경우 대비
	if (nullptr == Owner)	
	{
		Owner = Cast<ACCharacter>(TryGetPawnOwner());
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

	// TODO 활 애님
}

void UCAnimInstance_Character::OnWeaponTypeChanged(EWeaponType InPrevType, EWeaponType InNewType)
{
	WeaponType = InNewType;
	WeaponPrevType = InPrevType;
}

void UCAnimInstance_Character::OnStateTypeChanged(EStateType InPrevType, EStateType InNewType)
{
	StateType = InNewType;
	StatePrevType = InPrevType;
}
