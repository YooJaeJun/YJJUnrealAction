#include "Weapons/Skills/CSkill_Common.h"
#include "Global.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CAct.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"

void UCSkill_Common::Pressed()
{
	CheckNull(StateComp);
	CheckFalse(StateComp->IsIdleMode());
	CheckTrue(StateComp->IsSkillMode());

	Super::Pressed();

	StateComp->SetActMode();
	StateComp->OnSkillMode();

	ActData.Act(Owner);
}

void UCSkill_Common::Begin_Skill_Implementation()
{
	Super::Begin_Skill_Implementation();

	CheckNull(Attachment);

	if (Attachment->OnAttachmentBeginOverlap.IsBound())
		Attachment->OnAttachmentBeginOverlap.Remove(Act, "OnAttachmentBeginOverlap");

	Attachment->OnAttachmentBeginOverlap.AddUniqueDynamic(this, &UCSkill_Common::OnAttachmentBeginOverlap);
}

void UCSkill_Common::End_Skill_Implementation()
{
	Super::End_Skill_Implementation();

	if (Attachment->OnAttachmentBeginOverlap.IsBound())
		Attachment->OnAttachmentBeginOverlap.Remove(this, "OnAttachmentBeginOverlap");

	Attachment->OnAttachmentBeginOverlap.AddUniqueDynamic(Act, &UCAct::OnAttachmentBeginOverlap);

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

void UCSkill_Common::OnAttachmentBeginOverlap(
	ACCommonCharacter* InAttacker,
	AActor* InAttackCauser,
	ACCommonCharacter* InOther)
{
	CheckNull(InOther);

	//for (const TWeakObjectPtr<ACCommonCharacter> character : Hitted)
	//	CheckTrue(character == InOther);

	//Hitted.AddUnique(InOther);

	HitData.SendDamage(Owner, InAttackCauser, InOther);
}