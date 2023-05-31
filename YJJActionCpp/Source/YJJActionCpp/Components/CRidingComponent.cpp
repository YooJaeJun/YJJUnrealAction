#include "Components/CRidingComponent.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "Game/CGameMode.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Widgets/Interaction/CUserWidget_Interaction.h"
#include "Animals/CAnimal_AI.h"
#include "Engine/Texture2D.h"
#include "AIController.h"
#include "Components/CMovementComponent.h"
#include "Components/CStateComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UCRidingComponent::UCRidingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACAnimal_AI>(GetOwner());
}

void UCRidingComponent::BeginPlay()
{
	Super::BeginPlay();

	TWeakObjectPtr<ACGameMode> gameMode = Cast<ACGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	Hud = Cast<UCUserWidget_HUD>(gameMode->GetHUD());

	if (!!Hud)
	{
		Hud->SetChildren();
		Interaction = Hud->Interaction;

		CHelpers::LoadAsset<UTexture2D>(&InteractionKeyTexture,
			TEXT("Texture2D'/Game/Assets/Textures/ButtonPrompts/F_Key_Dark.F_Key_Dark'"));

		InteractionText = FText::FromString(TEXT("탑승"));

		Interaction->SetChildren(InteractionKeyTexture, InteractionText);
	}

	if (!!Owner.Get())
	{
		Mesh = Owner->GetMesh();
		SpringArm = Owner->GetSpringArm();
		Camera = Owner->GetCamera();
		MovementComp = Owner->MovementComp;
		StateComp = Owner->StateComp;

		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateLeft)] = Owner->GetMountLeftPoint();
		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateRight)] = Owner->GetMountRightPoint();
		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateBack)] = Owner->GetMountBackPoint();
		RidingPoints[static_cast<uint8>(ERidingPoint::Rider)] = Owner->GetRiderPoint();
		RidingPoints[static_cast<uint8>(ERidingPoint::Unmount)] = Owner->GetUnmountPoint();

		InteractionCollision = Owner->GetInteractionCollision();

		if (!!MovementComp)
			MovementComp->DisableControlRotation();

		if (!!SpringArm)
			Zooming = SpringArm->TargetArmLength;

		// TODO Eye
		// TODO Rider Info

		if (false == Owner->GetRiderPoint()->AttachToComponent(Owner->GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			"RiderPoint"))
			CLog::Log("RiderPoint Attach Fail");
	}
}

void UCRidingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (RidingState)
	{
	case ERidingState::None:
		if (!!Rider)
			RidingState = ERidingState::ToMountPoint;
		break;
	case ERidingState::ToMountPoint:
		Tick_ToMountPoint();
		break;
	case ERidingState::Mounting:
		Tick_Mounting();
		break;
	case ERidingState::MountingEnd:
		Tick_MountingEnd();
		break;
	case ERidingState::Riding:
		Tick_Riding();
		break;
	case ERidingState::Unmounting:
		Tick_Unmounting();
		break;
	case ERidingState::UnmountingEnd:
		Tick_UnmountingEnd();
		break;
	}
}

void UCRidingComponent::BeginOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	CheckNull(Interaction);

	const auto animal = Cast<ACAnimal_AI>(OtherActor);
	CheckTrue(!!animal);

	const auto interactor = Cast<ACCommonCharacter>(OtherActor);
	CheckNull(interactor);

	SetInteractableCharacter(interactor, Owner);
	SetInteractableCharacter(Owner, interactor);

	if (false == interactor->OnInteract.IsBound())
		interactor->OnInteract.AddDynamic(this, &UCRidingComponent::SetRider);

	Interaction->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCRidingComponent::EndOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CheckNull(Interaction);

	const auto animal = Cast<ACAnimal_AI>(OtherActor);
	CheckTrue(!!animal);

	const auto interactor = Cast<ACCommonCharacter>(OtherActor);
	CheckNull(interactor);

	SetInteractableCharacter(interactor, nullptr);
	SetInteractableCharacter(Owner, nullptr);

	if (!!interactor->OnInteract.IsBound())
		interactor->OnInteract.RemoveDynamic(this, &UCRidingComponent::SetRider);

	Interaction->SetVisibility(ESlateVisibility::Collapsed);
}

void UCRidingComponent::SetInteractableCharacter(TWeakObjectPtr<ACCommonCharacter> InCharacter, 
	const TWeakObjectPtr<ACCommonCharacter> InOtherCharacter)
{
	InCharacter->SetInteractor(InOtherCharacter.Get());
}

void UCRidingComponent::SetRider(ACCommonCharacter* InCharacter)
{
	Rider = InCharacter;
}

void UCRidingComponent::Tick_ToMountPoint()
{
	if (nullptr == RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)])
		CheckValidPoint();
	else
	{
		if (MoveToPoint(Rider, RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)]))
		{
			Rider->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
			Rider->GetCharacterMovement()->bEnablePhysicsInteraction = false;

			// TODO Controller Save
			// TODO Toggle Possess
			// TODO Toggle IK

			RidingState = ERidingState::Mounting;
		}
	}
}

void UCRidingComponent::Tick_Mounting()
{
	// 탑승 후 위치, 방향으로
	{
		const auto dir = UKismetMathLibrary::GetDirectionUnitVector(
			Rider->GetActorLocation(),
			RidingPoints[static_cast<uint8>(ERidingPoint::Rider)]->GetComponentLocation())
			* 50.0f;

		FVector targetLocation = dir + RidingPoints[static_cast<uint8>(ERidingPoint::Rider)]->GetComponentLocation();
		targetLocation.Z += 70.0f;

		const auto rotator = UKismetMathLibrary::FindLookAtRotation(
			Rider->GetActorLocation(),
			RidingPoints[static_cast<uint8>(ERidingPoint::Rider)]->GetComponentLocation());

		const FRotator targetRotation = FRotator(0, 0, rotator.Yaw + MountRotationZFactor);

		const TEnumAsByte<EMoveComponentAction::Type> eMoveAction = EMoveComponentAction::Type::Move;

		FLatentActionInfo latentInfo;
		latentInfo.CallbackTarget = this;

		UKismetSystemLibrary::MoveComponentTo(Rider->GetRootComponent(), targetLocation, targetRotation,
			false, true, 0.3f, false,
			eMoveAction, latentInfo);
	}

	// 탑승중애니 - 탑승후루프애니 블렌딩
	{
		Rider->GetMesh()->PlayAnimation(MountAnim, false);
		
		TEnumAsByte<EMoveComponentAction::Type> eMoveAction = EMoveComponentAction::Type::Move;

		FLatentActionInfo latentInfo;
		latentInfo.CallbackTarget = this;

		const auto targetLocation = RidingPoints[static_cast<uint8>(ERidingPoint::Rider)]->GetComponentLocation();
		const auto targetRotation = RidingPoints[static_cast<uint8>(ERidingPoint::Rider)]->GetComponentRotation();

		UKismetSystemLibrary::MoveComponentTo(Rider->GetRootComponent(), targetLocation, targetRotation,
			false, true, 0.2f, false,
			eMoveAction, latentInfo);

		if (false == Rider->GetMesh()->GetAnimInstance()->OnMontageEnded.IsBound())
			Rider->GetMesh()->GetAnimInstance()->OnMontageEnded.AddDynamic(
				this, &UCRidingComponent::AttachToRiderPoint);
	}

	RidingState = ERidingState::MountingEnd;
}

void UCRidingComponent::Tick_MountingEnd()
{
	Rider->StateComp->SetRideMode();
	RidingState = ERidingState::Riding;
}

void UCRidingComponent::Tick_Riding()
{
}

void UCRidingComponent::Tick_Unmounting()
{
}

void UCRidingComponent::Tick_UnmountingEnd()
{
}

void UCRidingComponent::CheckValidPoint()
{
	const float candidateLeft = UKismetMathLibrary::Vector_DistanceSquared(Rider->GetActorLocation(),
		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateLeft)]->GetComponentLocation());

	const float candidateRight = UKismetMathLibrary::Vector_DistanceSquared(Rider->GetActorLocation(),
		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateRight)]->GetComponentLocation());

	const float candidateBack = UKismetMathLibrary::Vector_DistanceSquared(Rider->GetActorLocation(),
		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateBack)]->GetComponentLocation());

	const float minCandidate = UKismetMathLibrary::Min(candidateLeft, UKismetMathLibrary::Min(candidateRight, candidateBack));

	if (UKismetMathLibrary::NearlyEqual_FloatFloat(minCandidate, candidateLeft, 5.0f))
	{
		MountDir = EDirection::Left;
		RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)] =
			RidingPoints[static_cast<uint8>(ERidingPoint::CandidateLeft)];

		CHelpers::LoadAsset<UAnimMontage>(&MountAnim,
			TEXT("AnimMontage'/Game/Character/AnimationMontages/Riding/Rider_Mount_Front_Left_Montage.Rider_Mount_Front_Left_Montage'"));

		MountRotationZFactor = 30.0f;
	}
	else if (UKismetMathLibrary::NearlyEqual_FloatFloat(minCandidate, candidateRight, 5.0f))
	{
		MountDir = EDirection::Right;
		RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)] =
			RidingPoints[static_cast<uint8>(ERidingPoint::CandidateRight)];

		CHelpers::LoadAsset<UAnimMontage>(&MountAnim,
			TEXT("AnimMontage'/Game/Character/AnimationMontages/Riding/Rider_Mount_Front_Right_Montage.Rider_Mount_Front_Right_Montage'"));

		MountRotationZFactor = -30.0f;
	}
	else if (UKismetMathLibrary::NearlyEqual_FloatFloat(minCandidate, candidateBack, 5.0f))
	{
		MountDir = EDirection::Back;
		RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)] =
			RidingPoints[static_cast<uint8>(ERidingPoint::CandidateBack)];

		CHelpers::LoadAsset<UAnimMontage>(&MountAnim,
			TEXT("AnimMontage'/Game/Character/AnimationMontages/Riding/Rider_Mount_Back_Montage.Rider_Mount_Back_Montage'"));

		MountRotationZFactor = 0.0f;
	}
}

bool UCRidingComponent::MoveToPoint(ACCommonCharacter* Char, const USceneComponent* To)
{
	bool reached = true;

	if (UKismetMathLibrary::NearlyEqual_FloatFloat(Char->GetActorLocation().X, To->GetComponentLocation().X, 15.0f) &&
		UKismetMathLibrary::NearlyEqual_FloatFloat(Char->GetActorLocation().Y, To->GetComponentLocation().Y, 15.0f))
	{
		Rider->GetCharacterMovement()->StopMovementImmediately();
		RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)] = nullptr;
		reached = true;
	}
	else
	{
		auto unitDirection = UKismetMathLibrary::GetDirectionUnitVector(Char->GetActorLocation(), To->GetComponentLocation());
		Rider->AddMovementInput(unitDirection, 1.0f);
		reached = false;
	}

	return reached;
}

void UCRidingComponent::AttachToRiderPoint(UAnimMontage* Anim, bool bInterrupted)
{
	CLog::Log("Attach");

}