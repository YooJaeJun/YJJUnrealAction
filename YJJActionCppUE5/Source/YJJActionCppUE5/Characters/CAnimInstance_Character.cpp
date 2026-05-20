#include "Characters/CAnimInstance_Character.h"
#include "Global.h"
#include "Characters/Animals/Dragon/CDragon.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Components/CFeetComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CFlyComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CWeaponComponent.h"
#include "Components/CMagicComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Weapons/CSkill.h"

namespace
{
	// CABP BlendList(CEWeaponType) 이 마법(EMagicType)·Shield/Dual 별칭을 하나의 열거로 기대할 때의 표시용.
	CEWeaponType ResolveLegacyAnimWeaponBlendType(UCWeaponComponent* WeaponComp)
	{
		if (false == IsValid(WeaponComp))
			return CEWeaponType::Unarmed;

		if (WeaponComp->IsMagicEquipped())
		{
			switch (WeaponComp->GetMagicEquipType())
			{
			case CEMagicType::Warp:
				return CEWeaponType::Warp;
			case CEMagicType::Around:
				return CEWeaponType::Around;
			case CEMagicType::FireBall:
				return CEWeaponType::Fireball;
			case CEMagicType::Bomb:
				return CEWeaponType::Bomb;
			case CEMagicType::Yondu:
				return CEWeaponType::Yondu;
			case CEMagicType::Unarmed:
			default:
				return CEWeaponType::Unarmed;
			}
		}

		const CEWeaponType phys = WeaponComp->GetLastCommittedPhysicalType();
		if (phys == CEWeaponType::Shield)
			return CEWeaponType::Guard;
		if (phys == CEWeaponType::Dual)
			return CEWeaponType::Sword_Hook;
		return phys;
	}
}

void UCAnimInstance_Character::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Owner = Cast<ACCommonCharacter>(TryGetPawnOwner());
	CheckNull(Owner);

	OwningCharacter = Owner.Get();
	StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());
	WeaponComponent = YJJHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	MagicComponent = YJJHelpers::GetComponent<UCMagicComponent>(Owner.Get());

	if (StateComp.IsValid())
		StateComp->OnStateTypeChanged.AddUniqueDynamic(this, &UCAnimInstance_Character::OnStateTypeChanged);

	if (IsValid(MagicComponent))
	{
		MagicComponent->OnMagicTypeChanged.AddUniqueDynamic(this, &UCAnimInstance_Character::OnMagicTypeChanged);
	}
	else if (IsValid(WeaponComponent))
	{
		WeaponComponent->OnMagicTypeChanged.AddUniqueDynamic(this, &UCAnimInstance_Character::OnMagicTypeChanged);
	}

	if (IsValid(WeaponComponent))
		WeaponComponent->OnWeaponTypeChanged.AddUniqueDynamic(this, &UCAnimInstance_Character::OnWeaponTypeChanged);

	MovementComp = YJJHelpers::GetComponent<UCMovementComponent>(Owner.Get());
	FlyComp = YJJHelpers::GetComponent<UCFlyComponent>(Owner.Get());
}

void UCAnimInstance_Character::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (false == Owner.IsValid())
		Owner = Cast<ACCommonCharacter>(TryGetPawnOwner());

	if (false == Owner.IsValid())
		return;

	OwningCharacter = Owner.Get();
	if (StateComp.IsValid())
		StateComponent = StateComp.Get();

	// ABP: ?? ? CurInteractingActor ?? RidingAnimal ?? ????(??? ???? ?��????? ????).
	if (IsValid(OwningCharacter))
		RidingAnimal = OwningCharacter->CurInteractingActor;

	UCapsuleComponent* ownerCapsule =
		IsValid(OwningCharacter) ? OwningCharacter->GetCapsuleComponent() : nullptr;

	const UWorld* world = GetWorld();
	const float worldDt = (IsValid(world) && world->GetDeltaSeconds() > 0.f) ? world->GetDeltaSeconds() : DeltaSeconds;

	Speed = Owner->GetVelocity().Size2D();

	const FVector velocity = Owner->GetVelocity();
	const FRotator velocityRot = UKismetMathLibrary::MakeRotFromX(velocity);
	const FRotator controlRotPawn = Owner->GetControlRotation();
	const FRotator deltaRot = UKismetMathLibrary::NormalizedDeltaRotator(velocityRot, controlRotPawn);

	PrevRotation = UKismetMathLibrary::RInterpTo(deltaRot, PrevRotation, worldDt, 25.f);
	Direction = static_cast<double>(PrevRotation.Yaw);

	// ABP: CurController ?? GetControlRotation ?? BF_Helpers.RotateFrom360To180 ?? ??????? NormalizeAxis.
	if (IsValid(OwningCharacter))
	{
		AController* curController = OwningCharacter->GetMyCurController().Get();
		if (IsValid(curController))
		{
			const FRotator ctrlRot = curController->GetControlRotation();
			Pitch = static_cast<double>(UKismetMathLibrary::NormalizeAxis(ctrlRot.Pitch));
			Neck_Yaw = static_cast<double>(UKismetMathLibrary::NormalizeAxis(ctrlRot.Yaw));
		}
	}

	Yaw = UKismetMathLibrary::FInterpTo(Yaw, Owner->GetBaseAimRotation().Yaw, DeltaSeconds, 25.f);

	if (StateComp.IsValid())
	{
		State = StateComp->GetCurMode();
		PrevState = StateComp->GetPrevMode();
		Falling = StateComp->IsFallMode();
		Hitting = StateComp->IsActMode();
	}

	bFalling = Falling;

	bAirborne = false;
	bJumpRising = false;
	if (ACharacter* ownerCharacter = Cast<ACharacter>(Owner.Get()))
	{
		if (UCharacterMovementComponent* characterMovement = ownerCharacter->GetCharacterMovement())
		{
			bAirborne = characterMovement->IsFalling();
			if (bAirborne)
				bJumpRising = characterMovement->Velocity.Z > JumpRisingZThreshold;
		}
	}

	if (IsValid(WeaponComponent))
	{
		MainWeaponType = WeaponComponent->GetLastCommittedPhysicalType();
		SubWeaponType = WeaponComponent->GetSubWeaponLaneType();
	}

	if (IsValid(MagicComponent))
		MagicType = MagicComponent->GetEquippedMagicSlot();
	else if (IsValid(WeaponComponent))
		MagicType = WeaponComponent->GetMagicEquipType();

	Bow_Aiming = false;
	if (IsValid(WeaponComponent))
	{
		UCSkill* skill0 = WeaponComponent->GetSkill(0);
		if (IsValid(skill0))
			Bow_Aiming = (MainWeaponType == CEWeaponType::Bow) && skill0->GetInAction();
	}

	bBowAiming = Bow_Aiming;

	WeaponType = ResolveLegacyAnimWeaponBlendType(WeaponComponent);

	Feet = false;
	if (IsValid(OwningCharacter))
	{
		UCFeetComponent* feetComp = YJJHelpers::GetComponent<UCFeetComponent>(OwningCharacter);
		if (IsValid(feetComp))
		{
			FeetData = feetComp->Data;
			Feet = true;
		}
	}

	const bool ridingByState = StateComp.IsValid() && StateComp->IsRidingMode();
	bRiding = ridingByState;

	RidingSpeed = 0.0;
	if (IsValid(RidingAnimal))
		RidingSpeed = static_cast<double>(RidingAnimal->GetVelocity().Size2D());

	ControllerNormalizedDirection = 0.0;
	ControllerDirection = 0.0;
	if (IsValid(RidingAnimal) && IsValid(OwningCharacter))
	{
		APawn* instigatorPawn = RidingAnimal->GetInstigator();
		if (IsValid(instigatorPawn) && IsValid(ownerCapsule))
		{
			const FVector instigatorForward = UKismetMathLibrary::GetForwardVector(instigatorPawn->GetControlRotation());
			const FVector capForward = ownerCapsule->GetForwardVector();
			const FVector capUp = ownerCapsule->GetUpVector();
			ControllerNormalizedDirection = static_cast<double>(YJJHelpers::GetLook(instigatorForward, capForward, capUp));
			ControllerDirection = ControllerNormalizedDirection * 180.0;
		}
	}

	bRealRiding = false;
	if (ACCommonCharacter* mountCommon = Cast<ACCommonCharacter>(RidingAnimal))
		bRealRiding = IsValid(mountCommon->CurInteractingActor);

	RidingAnimalFalling = false;
	if (bRealRiding)
	{
		if (ACharacter* mountChar = Cast<ACharacter>(RidingAnimal))
		{
			if (UCharacterMovementComponent* mountMove = mountChar->GetCharacterMovement())
				RidingAnimalFalling = mountMove->IsFalling();
		}
	}

	if (bRiding && bRealRiding)
	{
		if (ACPlayableCharacter* playable = Cast<ACPlayableCharacter>(Owner.Get()))
			LegIKAlpha = playable->LegIKAlpha;
		else
			LegIKAlpha = static_cast<double>(Owner->GetLegIKAlpha());
	}
	else
		LegIKAlpha = 1.0;

	if (false == bRiding)
	{
		ControllerNormalizedDirection = 0.0;
		ControllerDirection = 0.0;
		RidingAnimalFalling = false;
	}

	bRidingFalling = RidingAnimalFalling;

	bFootIK = true;
	bRidingIK = false;
	if (bRiding && IsValid(RidingAnimal))
	{
		ACharacter* mountCharacter = Cast<ACharacter>(RidingAnimal);
		if (mountCharacter && IsValid(ownerCapsule))
		{
			Look = YJJHelpers::GetLook(
				mountCharacter->GetCapsuleComponent()->GetForwardVector(),
				UKismetMathLibrary::GetForwardVector(Owner->GetControlRotation()),
				mountCharacter->GetCapsuleComponent()->GetUpVector()) * 180.f;
			bFootIK = false;
			bRidingIK = true;
		}
	}
	if (false == bRiding)
	{
		Forward = 0.f;
		Side = 0.f;
	}

	CheckNull(MovementComp);

	FVector forward = FVector::ZeroVector;
	FVector right = FVector::ZeroVector;

	const FRotator rotatorControllerForYaw(0.f, Owner->GetControlRotation().Yaw, 0.f);

	if (FlyComp.IsValid())
	{
		const TWeakObjectPtr<ACDragon> flyingCharacter = Cast<ACDragon>(Owner);
		CheckNull(flyingCharacter);
		forward = UKismetMathLibrary::GetForwardVector(rotatorControllerForYaw) * flyingCharacter->FlyComp->Forward;
		right = UKismetMathLibrary::GetRightVector(rotatorControllerForYaw) * flyingCharacter->FlyComp->Right;
	}
	else
	{
		forward = UKismetMathLibrary::GetForwardVector(rotatorControllerForYaw) * MovementComp->Forward;
		right = UKismetMathLibrary::GetRightVector(rotatorControllerForYaw) * MovementComp->Right;
	}

	const FVector current = (forward + right) * MovementComp->SpeedFactor;
	const float dotForward = UKismetMathLibrary::Dot_VectorVector(current, Owner->GetActorForwardVector());
	const float dotRight = UKismetMathLibrary::Dot_VectorVector(current, Owner->GetActorRightVector());
	Forward = UKismetMathLibrary::Lerp(Forward, dotForward, 0.05f);
	Side = UKismetMathLibrary::Lerp(Side, dotRight, 0.05f);
}

void UCAnimInstance_Character::OnStateTypeChanged(const CEStateType InPrevType, const CEStateType InNewType)
{
	State = InNewType;
	PrevState = InPrevType;
}

void UCAnimInstance_Character::OnWeaponTypeChanged(
	const CEWeaponType InPrevMainType,
	const CEWeaponType InNewMainType,
	const CEWeaponType InPrevSubType,
	const CEWeaponType InNewSubType)
{
	(void)InPrevMainType;
	(void)InPrevSubType;
	MainWeaponType = InNewMainType;
	SubWeaponType = InNewSubType;
	if (SubWeaponType == CEWeaponType::Max)
		SubWeaponType = CEWeaponType::Unarmed;

	WeaponType = ResolveLegacyAnimWeaponBlendType(WeaponComponent);
}

void UCAnimInstance_Character::OnMagicTypeChanged(CEMagicType InType, CEMagicType InPrevType)
{
	(void)InPrevType;
	MagicType = InType;
	WeaponType = ResolveLegacyAnimWeaponBlendType(WeaponComponent);
}
