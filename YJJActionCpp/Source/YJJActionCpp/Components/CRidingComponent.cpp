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

		CHelpers::LoadTextureFromPath<UTexture2D>(&InteractionKeyTexture,
			TEXT("Texture2D'/Game/Assets/Textures/ButtonPrompts/F_Key_Dark.F_Key_Dark'"));

		InteractionText = FText::FromString(TEXT("탑승"));

		Interaction->SetChildren(InteractionKeyTexture, InteractionText);
	}

	if (!!Owner)
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

		if (false == Owner->GetRiderPoint()->AttachToComponent(Owner->GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			"RiderPoint"))
			CLog::Log("RiderPoint Attach Fail");

		// TODO Eye
		// TODO Rider Info
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

	auto animal = Cast<ACAnimal_AI>(OtherActor);
	CheckTrue(!!animal);

	Rider = Cast<ACCommonCharacter>(OtherActor);
	CheckNull(Rider);

	SetInteractableCharacter(Rider, Owner);
	SetInteractableCharacter(Owner, Rider);

	if (false == Rider->OnInteract.IsBound())
		Rider->OnInteract.AddDynamic(Owner, &ACAnimal_AI::SetRider);

	Interaction->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCRidingComponent::EndOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CheckNull(Interaction);

	auto animal = Cast<ACAnimal_AI>(OtherActor);
	CheckTrue(!!animal);

	Rider = Cast<ACCommonCharacter>(OtherActor);
	CheckNull(Rider);

	SetInteractableCharacter(Rider, nullptr);
	SetInteractableCharacter(Owner, nullptr);

	if (!!Rider->OnInteract.IsBound())
		Rider->OnInteract.RemoveDynamic(Owner, &ACAnimal_AI::SetRider);

	Interaction->SetVisibility(ESlateVisibility::Collapsed);
}

void UCRidingComponent::SetInteractableCharacter(TWeakObjectPtr<ACCommonCharacter> InCharacter, 
	const TWeakObjectPtr<ACCommonCharacter> InOtherCharacter)
{
	InCharacter->SetInteractor(InOtherCharacter.Get());
}

void UCRidingComponent::Tick_ToMountPoint()
{
	if (nullptr == RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)])
		CheckValidPoint();
	else
	{
		MoveToPoint(Rider, RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)]);


	}
}

void UCRidingComponent::Tick_Mounting()
{
}

void UCRidingComponent::Tick_MountingEnd()
{
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
	float candidate1 = UKismetMathLibrary::Vector_DistanceSquared(Rider->GetActorLocation(),
		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateLeft)]->GetComponentLocation());

	float candidate2 = UKismetMathLibrary::Vector_DistanceSquared(Rider->GetActorLocation(),
		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateRight)]->GetComponentLocation());

	float candidate3 = UKismetMathLibrary::Vector_DistanceSquared(Rider->GetActorLocation(),
		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateBack)]->GetComponentLocation());

	float minCandidate = UKismetMathLibrary::Min(candidate1, UKismetMathLibrary::Min(candidate2, candidate3));
}

bool UCRidingComponent::MoveToPoint(ACCommonCharacter* Char, const USceneComponent* To)
{
	bool reached = true;

	//

	return reached;
}
