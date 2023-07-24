#include "Weapons/Acts/CAct_Yondu.h"
#include "Global.h"
#include "Components/CStateComponent.h"
#include "Weapons/AddOns/SkillColliders/CSkillCollider_Yondu.h"

UCAct_Yondu::UCAct_Yondu()
{
	CheckFalse(ActDatas.Num() > 0);
	CheckFalse(StateComp->IsIdleMode());

	Super::Act();

	ActDatas[0].Act(Owner);
}

void UCAct_Yondu::Act()
{
	Super::Act();

	FActorSpawnParameters params;
	params.Owner = Owner.Get();
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Yondu = Owner->GetWorld()->SpawnActor<ACSkillCollider_Yondu>(YonduClass, params);
}
