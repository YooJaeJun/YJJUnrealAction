#include "Components/CCamComponent.h"
#include "Global.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CTargetingComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CWeaponComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Animals/CAnimal_AI.h"
#include "GameFramework/CharacterMovementComponent.h"

UCCamComponent::UCCamComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCCamComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Owner.IsValid())
	{
		Zooming = Owner->GetSpringArm()->TargetArmLength;
		TargetingComp = Owner->GetTargetingComp();
		MovementComp = YJJHelpers::GetComponent<UCMovementComponent>(Owner.Get());;
	}
}

void UCCamComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const TWeakObjectPtr<UCWeaponComponent> weaponComp = 
		YJJHelpers::GetComponent<UCWeaponComponent>(Owner.Get());
	CheckNull(weaponComp);
	CheckTrue(weaponComp->IsBowMode());

	if (Owner.IsValid())
		TargetArmLength = Owner->GetSpringArm()->TargetArmLength;

	// Zoom
	if (UKismetMathLibrary::NearlyEqual_FloatFloat(Zooming, TargetArmLength, 0.1f))
		return;

	TargetArmLength = UKismetMathLibrary::FInterpTo(
		TargetArmLength, 
		Zooming,
		UGameplayStatics::GetWorldDeltaSeconds(GetWorld()),
		ZoomData.InterpSpeed);

	if (Owner.IsValid())
		Owner->GetSpringArm()->TargetArmLength = TargetArmLength;
}

void UCCamComponent::InputAxis_HorizontalLook(const float InAxis)
{
	CheckTrue(bFixedCamera);
	Owner->AddControllerYawInput(InAxis * HorizontalLook * GetWorld()->GetDeltaSeconds());
}

void UCCamComponent::InputAxis_VerticalLook(const float InAxis)
{
	CheckTrue(bFixedCamera);
	Owner->AddControllerPitchInput(InAxis * VerticalLook * GetWorld()->GetDeltaSeconds());
}

void UCCamComponent::InputAxis_Zoom(const float InAxis)
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

void UCCamComponent::EnableControlRotation() const
{
	Owner->bUseControllerRotationYaw = true;
	Owner->GetCharacterMovement()->bOrientRotationToMovement = false;
}

void UCCamComponent::DisableControlRotation() const
{
	Owner->bUseControllerRotationYaw = false;
	Owner->GetCharacterMovement()->bOrientRotationToMovement = true;
}

void UCCamComponent::EnableTopViewCamera() const
{
	if (OnEnableTopViewCam.IsBound())
		OnEnableTopViewCam.Broadcast();
}

void UCCamComponent::DisableTopViewCamera() const
{
	if (OnDisableTopViewCam.IsBound())
		OnDisableTopViewCam.Broadcast();
}