#include "Components/CZoomComponent.h"
#include "Global.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CTargetingComponent.h"
#include "Components/CMovementComponent.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Characters/Animals/CAnimal_AI.h"

UCZoomComponent::UCZoomComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Player = Cast<ACPlayableCharacter>(GetOwner());
	Animal = Cast<ACAnimal_AI>(GetOwner());
}

void UCZoomComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!!Player.Get())
	{
		Zooming = Player->GetSpringArm()->TargetArmLength;
		TargetingComp = Player->GetTargetingComp();
		MovementComp = Player->MovementComp;
	}
	else if (!!Animal.Get())
	{
		Zooming = Animal->GetSpringArm()->TargetArmLength;
		TargetingComp = Animal->GetTargetingComp();
		MovementComp = Animal->MovementComp;
	}
}

void UCZoomComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!!Player.Get())
		TargetArmLength = Player->GetSpringArm()->TargetArmLength;
	else if (!!Animal.Get())
		TargetArmLength = Animal->GetSpringArm()->TargetArmLength;


	if (UKismetMathLibrary::NearlyEqual_FloatFloat(
		Zooming, TargetArmLength, 0.1f))
		return;

	TargetArmLength = UKismetMathLibrary::FInterpTo(
		TargetArmLength, Zooming,
		UGameplayStatics::GetWorldDeltaSeconds(GetWorld()),
		ZoomData.InterpSpeed);


	if (!!Player.Get())
		Player->GetSpringArm()->TargetArmLength = TargetArmLength;
	else if (!!Animal.Get())
		Animal->GetSpringArm()->TargetArmLength = TargetArmLength;
}

void UCZoomComponent::InputAxis_Zoom(const float InAxis)
{
	if (InAxis == 0.0f)
		return;

	if (!!TargetingComp.Get() && 
		!!TargetingComp->bTargeting)
	{
		TargetingComp->ChangeFocus(InAxis > 0.0f);
	}
	else if(false == MovementComp->GetFixedCamera())
	{
		Zooming = UKismetMathLibrary::Clamp(
			Zooming + InAxis * ZoomData.ZoomSpeed,
			ZoomData.MinRange,
			ZoomData.MaxRange);
	}
}