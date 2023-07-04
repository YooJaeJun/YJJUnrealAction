#include "Weapons/Skills/CSkill_Sword.h"
#include "Global.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CAct.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Weapons/AddOns/CMotionTrail.h"

void UCSkill_Sword::Pressed()
{
	CheckNull(StateComp);
	CheckFalse(StateComp->IsIdleMode());
	CheckTrue(StateComp->IsSkillMode());

	Super::Pressed();

	StateComp->SetActMode();
	StateComp->OnSkillMode();

	MotionTrail = CHelpers::PlayMotionTrail(MotionTrailClass, Owner);

	ActData.Act(Owner);
}

void UCSkill_Sword::Begin_Skill_Implementation()
{
	Super::Begin_Skill_Implementation();
	CheckNull(Attachment);
	Attachment->OnAttachmentBeginOverlap.Remove(Act.Get(), "OnAttachmentBeginOverlap");
	Attachment->OnAttachmentBeginOverlap.AddDynamic(this, &UCSkill_Sword::OnAttachmentBeginOverlap);

	bMoving = true;

	Start = Owner->GetActorLocation();
	End = Start + Owner->GetActorForwardVector() * Distance;

	const float radius = Owner->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float height = Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FRotator rotation = Owner->GetActorRotation();

	TArray<AActor*> ignores;
	ignores.Add(Owner.Get());

	TArray<FHitResult> hitResults;
	TArray<TEnumAsByte<EObjectTypeQuery>> objects;
	objects.Add(EObjectTypeQuery::ObjectTypeQuery3);

	UKismetSystemLibrary::BoxTraceMultiForObjects(
		Owner->GetWorld(), 
		Start, End,
		FVector(0, radius, height), 
		rotation, 
		objects, 
		false, 
		ignores, 
		EDrawDebugTrace::ForOneFrame, 
		hitResults, 
		true);

	for (const FHitResult& hitResult : hitResults)
	{
		const TWeakObjectPtr<ACCommonCharacter> character = Cast<ACCommonCharacter>(hitResult.GetActor());

		if (!!character.Get())
		{
			character->GetCapsuleComponent()->SetCollisionProfileName("Skill");

			Overlapped.Add(character);
		}
	}

	FHitResult lineHitResult;
	UKismetSystemLibrary::LineTraceSingle(
		Owner->GetWorld(),
		Start, End,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ignores,
		DrawDebug,
		lineHitResult,
		true);

	if (lineHitResult.bBlockingHit)
	{
		const FVector direction = (End - Start).GetSafeNormal2D();
		End = lineHitResult.Location - (direction * radius * 2);

		if (DrawDebug == EDrawDebugTrace::ForDuration)
			DrawDebugSphere(
				Owner->GetWorld(),
				End,
				radius * 2,
				20,
				FColor::Magenta,
				true,
				2);
	}

	if (DrawDebug == EDrawDebugTrace::ForDuration)
		DrawDebugDirectionalArrow(
			Owner->GetWorld(),
			Start, End,
			25,
			FColor::Green,
			true,
			5,
			0,
			3);
}

void UCSkill_Sword::End_Skill_Implementation()
{
	Super::End_Skill_Implementation();

	Attachment->OnAttachmentBeginOverlap.Remove(this, "OnAttachmentBeginOverlap");
	Attachment->OnAttachmentBeginOverlap.AddDynamic(Act.Get(), &UCAct::OnAttachmentBeginOverlap);

	bMoving = false;

	StateComp->SetIdleMode();
	StateComp->OffSkillMode();

	MovementComp->Move();
	MovementComp->UnFixCamera();

	for (const TWeakObjectPtr<ACCommonCharacter> character : Overlapped)
		character->GetCapsuleComponent()->SetCollisionProfileName("Pawn");

	Overlapped.Empty();
	Hitted.Empty();

	if (!!MotionTrail.Get())
		MotionTrail->Destroy();
}

void UCSkill_Sword::Tick_Implementation(float InDeltaTime)
{
	Super::Tick_Implementation(InDeltaTime);

	CheckFalse(bMoving);

	const FVector location = Owner->GetActorLocation();
	const float radius = Owner->GetCapsuleComponent()->GetScaledCapsuleRadius();

	if (location.Equals(End, radius))
	{
		bMoving = false;
		Start = End = Owner->GetActorLocation();

		return;
	}
	const FVector direction = (End - Start).GetSafeNormal2D();
	Owner->AddActorWorldOffset(direction * Speed, true);
}

void UCSkill_Sword::OnAttachmentBeginOverlap(
	ACCommonCharacter* InAttacker,
	AActor* InAttackCauser,
	ACCommonCharacter* InOther)
{
	CheckNull(InOther);

	for (const TWeakObjectPtr<ACCommonCharacter> character : Hitted)
		CheckTrue(character == InOther);

	Hitted.AddUnique(InOther);

	HitData.SendDamage(Owner, InAttackCauser, InOther);
}
