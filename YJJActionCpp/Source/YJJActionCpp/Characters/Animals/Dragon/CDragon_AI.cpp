#include "Characters/Animals/Dragon/CDragon_AI.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/CHelpers.h"

ACDragon_AI::ACDragon_AI()
{
}

void ACDragon_AI::BeginPlay()
{
	Super::BeginPlay();

	SetZoomMaxRange(1000.0f);

	LandEffectScaleFactor = 2.0f;
}

void ACDragon_AI::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}