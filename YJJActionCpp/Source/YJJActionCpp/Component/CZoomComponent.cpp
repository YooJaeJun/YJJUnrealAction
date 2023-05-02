#include "Component/CZoomComponent.h"
#include "Global.h"
#include "Component/CMovementComponent.h"
#include "Component/CTargetingComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/CPlayableCharacter.h"

UCZoomComponent::UCZoomComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCZoomComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ACPlayableCharacter>(GetOwner());
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

void UCZoomComponent::OnZoom(const float InAxis)
{
	if (InAxis == 0.0f)
		return;

	CheckNull(Owner);

	if (!!Owner->TargetingComponent && 
		Owner->TargetingComponent->IsTargeting)
	{
		if (InAxis > 0.0f)
			Owner->TargetingComponent->ChangeFocus(true);
		else
			Owner->TargetingComponent->ChangeFocus(false);
	}
	else
	{
		if (false == Owner->MovementComponent->GetFixedCamera())
		{
			Zooming = UKismetMathLibrary::Clamp(
				Zooming + InAxis * ZoomData.ZoomSpeed,
				ZoomData.MinRange,
				ZoomData.MaxRange);
		}
	}
}