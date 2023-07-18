#include "Weapons/Acts/CAct_Bomb.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Weapons/AddOns/SkillColliders/CSkillCollider_Bomb.h"

void UCAct_Bomb::Act()
{
	CheckFalse(ActDatas.Num() > 0);
	CheckFalse(StateComp->IsIdleMode());

	Super::Act();

	ActDatas[0].Act(Owner);
}

void UCAct_Bomb::Begin_Act()
{
	Super::Begin_Act();

	FActorSpawnParameters params;
	params.Owner = Owner.Get();
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Owner->GetWorld()->SpawnActor<ACSkillCollider_Bomb>(BombClass, params);
}
