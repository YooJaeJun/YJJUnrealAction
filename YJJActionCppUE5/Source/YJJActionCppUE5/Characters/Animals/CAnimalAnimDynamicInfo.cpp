#include "Characters/Animals/CAnimalAnimDynamicInfo.h"
#include "GameFramework/Pawn.h"

ACAnimalAnimDynamicInfo::ACAnimalAnimDynamicInfo()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ACAnimalAnimDynamicInfo::Tick_Info_Implementation(APawn* InOwner)
{
	(void)InOwner;
}
