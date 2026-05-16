#include "Characters/CAnimInstance_Character.h"
#include "Global.h"
#include "Animals/Dragon/CDragon_AI.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Components/CFeetComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CFlyComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CWeaponComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Weapons/CSkill.h"

void UCAnimInstance_Character::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Owner = Cast<ACCommonCharacter>(TryGetPawnOwner());
	CheckNull(Owner);

	Character = Owner.Get();
	StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());
	WeaponComponent = YJJHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	// MagicComponent: BP ?? MagicComponent_C ?? UCLASS ?? ?????? ?? ???? ��???OnMagicTypeChanged ???��?.

	if (StateComp.IsValid())
		StateComp->OnStateTypeChanged.AddUniqueDynamic(this, &UCAnimInstance_Character::OnStateTypeChanged);

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

	Character = Owner.Get();
	if (StateComp.IsValid())
		StateComponent = StateComp.Get();

	// ABP: ?? ? CurInteractingActor ?? RidingAnimal ?? ????(??? ???? ?��????? ????).
	if (IsValid(Character))
		RidingAnimal = Character->CurInteractingActor;

	UCapsuleComponent* ownerCapsule = IsValid(Character) ? Character->GetCapsuleComponent() : nullptr;

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
	if (IsValid(Character))
	{
		AController* curController = Character->GetMyCurController().Get();
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

	if (IsValid(WeaponComponent))
	{
		MainWeaponType = WeaponComponent->GetType();
		const CEWeaponType prevW = WeaponComponent->GetPrevType();
		SubWeaponType = (prevW == CEWeaponType::Max) ? CEWeaponType::Unarmed : prevW;
		MagicType = MainWeaponType;
	}

	Bow_Aiming = false;
	if (IsValid(WeaponComponent))
	{
		UCSkill* skill0 = WeaponComponent->GetSkill(0);
		if (IsValid(skill0))
			Bow_Aiming = (MainWeaponType == CEWeaponType::Bow) && skill0->GetInAction();
	}

	Feet = false;
	if (IsValid(Character))
	{
		UCFeetComponent* feetComp = YJJHelpers::GetComponent<UCFeetComponent>(Character);
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
	if (IsValid(RidingAnimal) && IsValid(Character))
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
		const TWeakObjectPtr<ACDragon_AI> flyingCharacter = Cast<ACDragon_AI>(Owner);
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

void UCAnimInstance_Character::OnWeaponTypeChanged(const CEWeaponType InPrevType, const CEWeaponType InNewType)
{
	MainWeaponType = InNewType;
	SubWeaponType = InPrevType;
	if (SubWeaponType == CEWeaponType::Max)
		SubWeaponType = CEWeaponType::Unarmed;
	MagicType = MainWeaponType;
}
