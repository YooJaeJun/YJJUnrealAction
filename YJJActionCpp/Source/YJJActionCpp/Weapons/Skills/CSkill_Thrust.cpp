#include "Weapons/Skills/CSkill_Thrust.h"
#include "Global.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CAct.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"

void UCSkill_Thrust::Pressed()
{
	CheckNull(StateComp);
	CheckFalse(StateComp->IsIdleMode());
	CheckTrue(StateComp->IsSkillMode());

	Super::Pressed();

	StateComp->SetActMode();
	StateComp->OnSkillMode();

	ActData.Act(Owner);
}

void UCSkill_Thrust::Begin_Skill_Implementation()
{
	Super::Begin_Skill_Implementation();

	CheckNull(Attachment);

	if (Attachment->OnAttachmentBeginOverlap.IsBound())
		Attachment->OnAttachmentBeginOverlap.Remove(Act, "OnAttachmentBeginOverlap");

	Attachment->OnAttachmentBeginOverlap.AddUniqueDynamic(this, &UCSkill_Thrust::OnAttachmentBeginOverlap);

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

void UCSkill_Thrust::End_Skill_Implementation()
{
	Super::End_Skill_Implementation();

	if (Attachment->OnAttachmentBeginOverlap.IsBound())
		Attachment->OnAttachmentBeginOverlap.Remove(this, "OnAttachmentBeginOverlap");

	Attachment->OnAttachmentBeginOverlap.AddUniqueDynamic(Act, &UCAct::OnAttachmentBeginOverlap);

	bMoving = false;

	CheckNull(StateComp);

	StateComp->SetIdleMode();
	StateComp->OffSkillMode();

	CheckNull(MovementComp);

	MovementComp->Move();
	MovementComp->UnFixCamera();

	for (const TWeakObjectPtr<ACCommonCharacter> character : Overlapped)
		character->GetCapsuleComponent()->SetCollisionProfileName("Pawn");

	Overlapped.Empty();
	Hitted.Empty();
}

void UCSkill_Thrust::Tick_Implementation(float InDeltaTime)
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

void UCSkill_Thrust::OnAttachmentBeginOverlap(
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
