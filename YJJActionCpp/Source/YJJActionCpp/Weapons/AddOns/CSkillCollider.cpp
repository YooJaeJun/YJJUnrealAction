#include "Weapons/AddOns/CSkillCollider.h"
#include "Global.h"

ACSkillCollider::ACSkillCollider()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACSkillCollider::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACSkillCollider::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

