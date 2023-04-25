#include "CCharacter.h"

ACCharacter::ACCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}