#include "Components/CZoomComponent.h"
#include "Global.h"
#include "Components/CMovementComponent.h"
#include "Components/CTargetingComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Characters/Player/CPlayableCharacter.h"

UCZoomComponent::UCZoomComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACPlayableCharacter>(GetOwner());
}

void UCZoomComponent::BeginPlay()
{
	Super::BeginPlay();
	CheckNull(Owner);

	Zooming = Owner->GetSpringArm()->TargetArmLength;
}

void UCZoomComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckNull(Owner);

	if (UKismetMathLibrary::NearlyEqual_FloatFloat(
		Zooming, Owner->GetSpringArm()->TargetArmLength, 0.1f))
		return;

	Owner->GetSpringArm()->TargetArmLength = UKismetMathLibrary::FInterpTo(
		Owner->GetSpringArm()->TargetArmLength,
		Zooming,
		UGameplayStatics::GetWorldDeltaSeconds(GetWorld()),
		ZoomData.InterpSpeed
	);
}

void UCZoomComponent::InputAxis_Zoom(const float InAxis)
{
	if (InAxis == 0.0f)
		return;

	CheckNull(Owner);

	if (!!Owner->GetTargetingComp() && 
		Owner->GetTargetingComp()->IsTargeting)
	{
		if (InAxis > 0.0f)
			Owner->GetTargetingComp()->ChangeFocus(true);
		else
			Owner->GetTargetingComp()->ChangeFocus(false);
	}
	else if(false == Owner->MovementComp->GetFixedCamera())
	{
		Zooming = UKismetMathLibrary::Clamp(
			Zooming + InAxis * ZoomData.ZoomSpeed,
			ZoomData.MinRange,
			ZoomData.MaxRange);
	}
}