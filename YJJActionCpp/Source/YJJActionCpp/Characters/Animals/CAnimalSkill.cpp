#include "Characters/Animals/CAnimalSkill.h"
#include "Global.h"

ACAnimalSkill::ACAnimalSkill()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACAnimalSkill::BeginPlay()
{
	Super::BeginPlay();
}

void ACAnimalSkill::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

