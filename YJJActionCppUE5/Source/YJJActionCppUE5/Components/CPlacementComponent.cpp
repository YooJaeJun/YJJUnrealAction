#include "Components/CPlacementComponent.h"
#include "Commons/CPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

UCPlacementComponent::UCPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCPlacementComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(false);
}

void UCPlacementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelPlacement();

	Super::EndPlay(EndPlayReason);
}

void UCPlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FTransform previewTransform;
	bPreviewValid = TracePlacement(previewTransform);

	if (bPreviewValid)
	{
		LastPreviewTransform = previewTransform;
		SetPreviewTransform(previewTransform);
	}
}

void UCPlacementComponent::StartPlacement(FName InItemID)
{
	if (InItemID == NAME_None)
		return;

	APawn* ownerPawn = Cast<APawn>(GetOwner());
	if (false == IsValid(ownerPawn) || false == ownerPawn->IsLocallyControlled())
		return;

	PendingItemID = InItemID;
	bPreviewValid = false;

	if (false == IsValid(PreviewActor) && IsValid(PreviewActorClass))
	{
		UWorld* world = GetWorld();
		if (IsValid(world))
		{
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = GetOwner();
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			PreviewActor = world->SpawnActor<AActor>(PreviewActorClass, FTransform::Identity, spawnParams);
			if (IsValid(PreviewActor))
			{
				PreviewActor->SetReplicates(false);
				PreviewActor->SetActorEnableCollision(false);
			}
		}
	}

	if (IsValid(PreviewActor))
		PreviewActor->SetActorHiddenInGame(false);

	SetComponentTickEnabled(true);
}

void UCPlacementComponent::ConfirmPlacement()
{
	if (PendingItemID == NAME_None || false == bPreviewValid)
		return;

	ACPlayerController* playerController = GetOwningPlayerController();
	if (false == IsValid(playerController))
		return;

	playerController->RequestPlacement(LastPreviewTransform, PendingItemID);

	CancelPlacement();
}

void UCPlacementComponent::CancelPlacement()
{
	SetComponentTickEnabled(false);

	PendingItemID = NAME_None;
	bPreviewValid = false;
	LastPreviewTransform = FTransform::Identity;

	if (IsValid(PreviewActor))
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

bool UCPlacementComponent::TracePlacement(FTransform& OutTransform) const
{
	ACPlayerController* playerController = GetOwningPlayerController();
	if (false == IsValid(playerController))
		return false;

	FVector viewLocation;
	FRotator viewRotation;
	playerController->GetPlayerViewPoint(viewLocation, viewRotation);

	const FVector traceStart = viewLocation;
	const FVector traceEnd = traceStart + viewRotation.Vector() * TraceDistance;

	FHitResult hitResult;
	FCollisionQueryParams queryParams(SCENE_QUERY_STAT(PlacementTrace), false, GetOwner());
	const bool bHit = GetWorld()->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, ECC_Visibility, queryParams);
	if (false == bHit)
		return false;

	OutTransform = FTransform(FRotator::ZeroRotator, hitResult.ImpactPoint, FVector::OneVector);

	return true;
}

ACPlayerController* UCPlacementComponent::GetOwningPlayerController() const
{
	const APawn* ownerPawn = Cast<APawn>(GetOwner());
	if (false == IsValid(ownerPawn))
		return nullptr;

	return Cast<ACPlayerController>(ownerPawn->GetController());
}

void UCPlacementComponent::SetPreviewTransform(const FTransform& InTransform)
{
	if (IsValid(PreviewActor))
		PreviewActor->SetActorTransform(InTransform);
}
