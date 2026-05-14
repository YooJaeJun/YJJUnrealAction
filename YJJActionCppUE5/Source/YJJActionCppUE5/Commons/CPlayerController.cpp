#include "Commons/CPlayerController.h"
#include "Buildings/CPlacedActor.h"
#include "Components/CInventoryComponent.h"
#include "Commons/CGameState.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Items/CWorldItemActor.h"

ACPlayerController::ACPlayerController()
{
	bReplicates = true;
	DefaultPlacedActorClass = ACPlacedActor::StaticClass();
}

void ACPlayerController::RequestPickup(ACWorldItemActor* WorldItem)
{
	if (HasAuthority())
	{
		Server_RequestPickup_Implementation(WorldItem);

		return;
	}

	Server_RequestPickup(WorldItem);
}

void ACPlayerController::RequestPlacement(const FTransform& RequestedTransform, const FName ItemID)
{
	if (HasAuthority())
	{
		Server_RequestPlacement_Implementation(RequestedTransform, ItemID);

		return;
	}

	Server_RequestPlacement(RequestedTransform, ItemID);
}

void ACPlayerController::Server_RequestPickup_Implementation(ACWorldItemActor* WorldItem)
{
	if (false == IsValid(WorldItem))
	{
		Client_NotifyServerActionResult(false, FName(TEXT("InvalidItem")));

		return;
	}

	const bool bSucceeded = WorldItem->TryPickup(this);
	Client_NotifyServerActionResult(bSucceeded, bSucceeded ? FName(TEXT("PickupSucceeded")) : FName(TEXT("PickupRejected")));
}

void ACPlayerController::Server_RequestPlacement_Implementation(FTransform RequestedTransform, FName ItemID)
{
	if (false == ValidatePlacementRequest(RequestedTransform, ItemID))
	{
		Client_NotifyServerActionResult(false, FName(TEXT("PlacementRejected")));

		return;
	}

	UCInventoryComponent* inventoryComp = FindInventoryComponent();
	if (false == IsValid(inventoryComp) || false == inventoryComp->RemoveItem(ItemID, 1))
	{
		Client_NotifyServerActionResult(false, FName(TEXT("MissingItem")));

		return;
	}

	UWorld* world = GetWorld();
	if (false == IsValid(world))
	{
		inventoryComp->AddItem(ItemID, 1);
		Client_NotifyServerActionResult(false, FName(TEXT("InvalidWorld")));

		return;
	}

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = GetPawn();
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	ACPlacedActor* placedActor = world->SpawnActor<ACPlacedActor>(DefaultPlacedActorClass, RequestedTransform, spawnParams);
	if (false == IsValid(placedActor))
	{
		inventoryComp->AddItem(ItemID, 1);
		Client_NotifyServerActionResult(false, FName(TEXT("SpawnFailed")));

		return;
	}

	placedActor->InitializePlacedActor(ItemID);
	placedActor->EnterPlacedDormancy();

	ACGameState* gameState = world->GetGameState<ACGameState>();
	if (IsValid(gameState))
		gameState->AddPlacedActorCount(1);

	Client_NotifyServerActionResult(true, FName(TEXT("PlacementSucceeded")));
}

void ACPlayerController::Client_NotifyServerActionResult_Implementation(bool bSucceeded, FName Reason)
{
	UE_LOG(LogTemp, Log, TEXT("Server action result: %s (%s)"), bSucceeded ? TEXT("Success") : TEXT("Failed"), *Reason.ToString());
}

bool ACPlayerController::ValidatePlacementRequest(const FTransform& RequestedTransform, const FName ItemID) const
{
	if (ItemID == NAME_None || false == IsValid(DefaultPlacedActorClass))
		return false;

	const APawn* controlledPawn = GetPawn();
	if (false == IsValid(controlledPawn))
		return false;

	const float distanceSquared = FVector::DistSquared(controlledPawn->GetActorLocation(), RequestedTransform.GetLocation());
	if (distanceSquared > FMath::Square(MaxPlacementDistance))
		return false;

	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return false;

	const FCollisionShape collisionShape = FCollisionShape::MakeSphere(PlacementCollisionRadius);
	const bool bBlocked = world->OverlapBlockingTestByChannel(
		RequestedTransform.GetLocation(),
		RequestedTransform.GetRotation(),
		ECC_WorldStatic,
		collisionShape);

	if (bBlocked)
		return false;

	UCInventoryComponent* inventoryComp = FindInventoryComponent();

	return IsValid(inventoryComp) && inventoryComp->HasItem(ItemID, 1);
}

UCInventoryComponent* ACPlayerController::FindInventoryComponent() const
{
	const APawn* controlledPawn = GetPawn();
	if (IsValid(controlledPawn))
	{
		UCInventoryComponent* pawnInventoryComp = controlledPawn->FindComponentByClass<UCInventoryComponent>();
		if (IsValid(pawnInventoryComp))
			return pawnInventoryComp;
	}

	if (IsValid(PlayerState))
		return PlayerState->FindComponentByClass<UCInventoryComponent>();

	return nullptr;
}
