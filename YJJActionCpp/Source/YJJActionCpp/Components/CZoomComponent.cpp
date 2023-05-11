#include "Components/CZoomComponent.h"
#include "Global.h"
#include "Components/CMovementComponent.h"
#include "Components/CTargetingComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/CPlayableCharacter.h"

UCZoomComponent::UCZoomComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACPlayableCharacter>(GetOwner());
}

void UCZoomComponent::BeginPlay()
{
	Super::BeginPlay();
	CheckNull(Owner);

	Zooming = Owner->SpringArm->TargetArmLength;
}

void UCZoomComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckNull(Owner);

	if (UKismetMathLibrary::NearlyEqual_FloatFloat(
		Zooming, Owner->SpringArm->TargetArmLength, 0.1f))
		return;

	Owner->SpringArm->TargetArmLength = UKismetMathLibrary::FInterpTo(
		Owner->SpringArm->TargetArmLength,
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

	if (!!Owner->TargetingComp && 
		Owner->TargetingComp->IsTargeting)
	{
		if (InAxis > 0.0f)
			Owner->TargetingComp->ChangeFocus(true);
		else
			Owner->TargetingComp->ChangeFocus(false);
	}
	else
	{
		if (false == Owner->MovementComp->GetFixedCamera())
		{
			Zooming = UKismetMathLibrary::Clamp(
				Zooming + InAxis * ZoomData.ZoomSpeed,
				ZoomData.MinRange,
				ZoomData.MaxRange);
		}
	}
}