#include "Weapons/Skills/CSkill_Aura.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CCamComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CAct.h"
#include "Weapons/AddOns/SkillColliders/CSkillCollider_Aura.h"

void UCSkill_Aura::Pressed()
{
	CheckNull(StateComp);
	CheckFalse(StateComp->IsIdleMode());
	CheckTrue(StateComp->IsSkillMode());

	Super::Pressed();

	StateComp->SetActMode();
	StateComp->OnSkillMode();

	ActData.Act(Owner);
}

void UCSkill_Aura::Begin_Skill_Implementation()
{
	Super::Begin_Skill_Implementation();

	FActorSpawnParameters params;
	params.Owner = Owner.Get();
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FTransform transform;
	transform.SetLocation(Owner->GetActorLocation());
	transform.AddToTranslation(AuraLocation);
	transform.SetRotation(FQuat(Owner->GetActorRotation()));

	Owner->GetWorld()->SpawnActor<ACSkillCollider_Aura>(AuraClass, transform, params);
}

void UCSkill_Aura::End_Skill_Implementation()
{
	Super::End_Skill_Implementation();

	CheckNull(StateComp);

	StateComp->SetIdleMode();
	StateComp->OffSkillMode();

	CheckNull(MovementComp);

	MovementComp->Move();
	CamComp->DisableFixedCamera();
}
