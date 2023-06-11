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

	OwnerPlayer = Cast<ACPlayableCharacter>(GetOwner());
	OwnerAnimal = Cast<ACAnimal_AI>(GetOwner());
}

void UCZoomComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!!OwnerPlayer.Get())
	{
		Zooming = OwnerPlayer->GetSpringArm()->TargetArmLength;
		TargetingComp = OwnerPlayer->GetTargetingComp();
		MovementComp = OwnerPlayer->MovementComp;
	}
	else if (!!OwnerAnimal.Get())
	{
		Zooming = OwnerAnimal->GetSpringArm()->TargetArmLength;
		TargetingComp = OwnerAnimal->GetTargetingComp();
		MovementComp = OwnerAnimal->MovementComp;
	}
}

void UCZoomComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!!OwnerPlayer.Get())
		TargetArmLength = OwnerPlayer->GetSpringArm()->TargetArmLength;
	else if (!!OwnerAnimal.Get())
		TargetArmLength = OwnerAnimal->GetSpringArm()->TargetArmLength;


	if (UKismetMathLibrary::NearlyEqual_FloatFloat(
		Zooming, TargetArmLength, 0.1f))
		return;

	TargetArmLength = UKismetMathLibrary::FInterpTo(
		TargetArmLength, Zooming,
		UGameplayStatics::GetWorldDeltaSeconds(GetWorld()),
		ZoomData.InterpSpeed);


	if (!!OwnerPlayer.Get())
		OwnerPlayer->GetSpringArm()->TargetArmLength = TargetArmLength;
	else if (!!OwnerAnimal.Get())
		OwnerAnimal->GetSpringArm()->TargetArmLength = TargetArmLength;
}

void UCZoomComponent::InputAxis_Zoom(const float InAxis)
{
	CheckTrue(InAxis == 0.0f);

	if (!!TargetingComp.Get() && 
		!!TargetingComp->bTargeting)
	{
		TargetingComp->ChangeFocus(InAxis <= 0.0f);
	}
	else if(false == MovementComp->GetFixedCamera())
	{
		Zooming = UKismetMathLibrary::Clamp(
			Zooming + InAxis * ZoomData.ZoomSpeed,
			ZoomData.MinRange,
			ZoomData.MaxRange);
	}
}