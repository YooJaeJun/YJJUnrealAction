#include "Components/CCameraComponent.h"
#include "Global.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CTargetingComponent.h"
#include "Components/CMovementComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Animals/CAnimal_AI.h"
#include "GameFramework/CharacterMovementComponent.h"

UCCameraComponent::UCCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Owner.IsValid())
	{
		Zooming = Owner->GetSpringArm()->TargetArmLength;
		TargetingComp = Owner->GetTargetingComp();
		MovementComp = YJJHelpers::GetComponent<UCMovementComponent>(Owner.Get());;
	}
}

void UCCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Owner.IsValid())
		TargetArmLength = Owner->GetSpringArm()->TargetArmLength;


	if (UKismetMathLibrary::NearlyEqual_FloatFloat(
		Zooming, TargetArmLength, 0.1f))
		return;

	TargetArmLength = UKismetMathLibrary::FInterpTo(
		TargetArmLength, Zooming,
		UGameplayStatics::GetWorldDeltaSeconds(GetWorld()),
		ZoomData.InterpSpeed);


	if (Owner.IsValid())
		Owner->GetSpringArm()->TargetArmLength = TargetArmLength;
}

void UCCameraComponent::InputAxis_HorizontalLook(const float InAxis)
{
	CheckTrue(bFixedCamera);
	Owner->AddControllerYawInput(InAxis * HorizontalLook * GetWorld()->GetDeltaSeconds());
}

void UCCameraComponent::InputAxis_VerticalLook(const float InAxis)
{
	CheckTrue(bFixedCamera);
	Owner->AddControllerPitchInput(InAxis * VerticalLook * GetWorld()->GetDeltaSeconds());
}

void UCCameraComponent::InputAxis_Zoom(const float InAxis)
{
	CheckTrue(InAxis == 0.0f);

	if (TargetingComp.IsValid() && 
		true == TargetingComp->bTargeting)
	{
		TargetingComp->ChangeFocus(InAxis <= 0.0f);
	}
	else if(false == GetFixedCamera())
	{
		Zooming = UKismetMathLibrary::Clamp(
			Zooming + InAxis * ZoomData.ZoomSpeed,
			ZoomData.MinRange,
			ZoomData.MaxRange);
	}
}

void UCCameraComponent::EnableControlRotation() const
{
	Owner->bUseControllerRotationYaw = true;
	Owner->GetCharacterMovement()->bOrientRotationToMovement = false;
}

void UCCameraComponent::DisableControlRotation() const
{
	Owner->bUseControllerRotationYaw = false;
	Owner->GetCharacterMovement()->bOrientRotationToMovement = true;
}