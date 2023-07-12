#include "Weapons/Acts/CAct_Around.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Weapons/AddOns/SkillColliders/CSkillCollider_Rotate.h"

void UCAct_Around::Act()
{
	CheckFalse(ActDatas.Num() > 0);
	CheckFalse(StateComp->IsIdleMode());

	Super::Act();

	ActDatas[0].Act(Owner);
}

void UCAct_Around::Begin_Act()
{
	Super::Begin_Act();

	const int32 index = 
		UKismetMathLibrary::RandomIntegerInRange(0, RotateClasses.Num() - 1);

	FActorSpawnParameters params;
	params.Owner = Owner.Get();
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Owner->GetWorld()->SpawnActor<ACSkillCollider_Rotate>(RotateClasses[index], params);
}