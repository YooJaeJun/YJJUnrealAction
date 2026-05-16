#include "Characters/Animals/CAnimalAnimDynamicInfo.h"

#include "Characters/CAnimInstance_Character.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CFeetComponent.h"
#include "Components/CStateComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Utilities/YJJHelpers.h"

ACAnimalAnimDynamicInfo::ACAnimalAnimDynamicInfo()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ACAnimalAnimDynamicInfo::BP_CallTick_Info(APawn* InOwner)
{
	Tick_Info(InOwner);
}

void ACAnimalAnimDynamicInfo::Tick_Info_Implementation(APawn* InOwner)
{
	Character = Cast<ACCommonCharacter>(InOwner);
	if (!IsValid(Character))
		return;

	USkeletalMeshComponent* mesh = Character->GetMesh();
	if (!IsValid(mesh))
		return;

	bool movementFalling = false;
	ACharacter* asCharacter = Cast<ACharacter>(Character.Get());
	if (IsValid(asCharacter))
	{
		UCharacterMovementComponent* move = asCharacter->GetCharacterMovement();
		if (IsValid(move))
			movementFalling = move->IsFalling();
	}

	TObjectPtr<UCStateComponent> stateTp = YJJHelpers::GetComponent<UCStateComponent>(Character);
	UCStateComponent* stateComp = stateTp.Get();
	UAnimInstance* animBase = mesh->GetAnimInstance();
	UCAnimInstance_Character* anim = Cast<UCAnimInstance_Character>(animBase);

	if (!IsValid(anim))
	{
		Speed = static_cast<double>(Character->GetVelocity().Size2D());
		Falling = movementFalling;
		Riding = IsValid(stateComp) && stateComp->IsRealRiding();
		Feet = false;
		LeftFoot = 0.0;
		RightFoot = 0.0;
		LeftHand = 0.0;
		RightHand = 0.0;
		Pelvis = 0.0;
		GroundSlope = FRotator::ZeroRotator;
		Index = 0;
		return;
	}

	Speed = static_cast<double>(anim->Speed);
	Direction = anim->Direction;
	ControllerDirection = anim->ControllerNormalizedDirection;
	ControllerDirection180 = anim->ControllerDirection;
	Side = static_cast<double>(anim->Side);
	Forward = static_cast<double>(anim->Forward);
	PrevRotation = anim->PrevRotation;
	Falling = movementFalling;
	Riding = IsValid(stateComp) && stateComp->IsRealRiding();

	TObjectPtr<UCFeetComponent> feetTp = YJJHelpers::GetComponent<UCFeetComponent>(Character);
	UCFeetComponent* feetComp = feetTp.Get();
	if (IsValid(feetComp))
	{
		FeetData = feetComp->Data;
		Feet = true;
		LeftFoot = feetComp->LeftFoot;
		RightFoot = feetComp->RightFoot;
		LeftHand = feetComp->LeftHand;
		RightHand = feetComp->RightHand;
		Pelvis = feetComp->Pelvis;
		GroundSlope = feetComp->GroundSlope;
	}
	else
	{
		Feet = false;
	}

	if (Falling)
		Feet = false;

	Index = 0;
}

bool ACAnimalAnimDynamicInfo::FromMoveToJumpStart() const
{
	return Falling;
}

bool ACAnimalAnimDynamicInfo::FromFallingToMove() const
{
	return false == Falling;
}

bool ACAnimalAnimDynamicInfo::FromMoveToFalling() const
{
	return Falling;
}

bool ACAnimalAnimDynamicInfo::FromMoveToRotate() const
{
	return true;
}

bool ACAnimalAnimDynamicInfo::FromRotateToJumpStart() const
{
	return Falling;
}
