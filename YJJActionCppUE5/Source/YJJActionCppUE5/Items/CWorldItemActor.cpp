#include "Items/CWorldItemActor.h"
#include "Characters/CCommonCharacter.h"
#include "Commons/CGameState.h"
#include "Commons/CPlayerController.h"
#include "Components/CInventoryComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

ACWorldItemActor::ACWorldItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);
	SetNetCullDistanceSquared(FMath::Square(10000.0f));
	NetDormancy = DORM_Awake;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(SceneRoot);
	MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(SceneRoot);
	InteractionSphere->SetSphereRadius(PickupDistance);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ACWorldItemActor::BeginPlay()
{
	Super::BeginPlay();

	ACGameState* gameState = GetWorld()->GetGameState<ACGameState>();
	if (HasAuthority() && IsValid(gameState))
		gameState->AddWorldItemCount(1);
}

void ACWorldItemActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ACGameState* gameState = GetWorld()->GetGameState<ACGameState>();
	if (HasAuthority() && IsValid(gameState))
		gameState->AddWorldItemCount(-1);

	Super::EndPlay(EndPlayReason);
}

void ACWorldItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACWorldItemActor, ItemID);
	DOREPLIFETIME(ACWorldItemActor, Quantity);
	DOREPLIFETIME(ACWorldItemActor, bClaimed);
}

void ACWorldItemActor::Interact(TObjectPtr<ACCommonCharacter> InteractingActor)
{
	if (false == IsValid(InteractingActor))
		return;

	TObjectPtr<ACPlayerController> playerController = Cast<ACPlayerController>(InteractingActor->GetController());
	if (IsValid(playerController))
		playerController->RequestPickup(this);
}

bool ACWorldItemActor::TryPickup(AController* RequestController)
{
	if (false == HasAuthority())
		return false;

	if (false == IsValid(RequestController))
		return false;

	const APawn* requestPawn = RequestController->GetPawn();
	if (false == CanPickupBy(requestPawn))
		return false;

	UCInventoryComponent* inventoryComp = FindInventoryComponent(RequestController);
	if (false == IsValid(inventoryComp))
		return false;

	if (false == inventoryComp->CanAddItem(ItemID, Quantity))
		return false;

	bClaimed = true;

	if (false == inventoryComp->AddItem(ItemID, Quantity))
	{
		bClaimed = false;

		return false;
	}

	Destroy();

	return true;
}

bool ACWorldItemActor::CanPickupBy(const APawn* RequestPawn) const
{
	if (bClaimed || ItemID == NAME_None || Quantity <= 0)
		return false;

	if (false == IsValid(RequestPawn))
		return false;

	const float distanceSquared = FVector::DistSquared(RequestPawn->GetActorLocation(), GetActorLocation());

	return distanceSquared <= FMath::Square(PickupDistance);
}

UCInventoryComponent* ACWorldItemActor::FindInventoryComponent(const AController* RequestController) const
{
	if (false == IsValid(RequestController))
		return nullptr;

	const APawn* requestPawn = RequestController->GetPawn();
	if (IsValid(requestPawn))
	{
		UCInventoryComponent* pawnInventoryComp = requestPawn->FindComponentByClass<UCInventoryComponent>();
		if (IsValid(pawnInventoryComp))
			return pawnInventoryComp;
	}

	APlayerState* playerState = RequestController->PlayerState;
	if (IsValid(playerState))
		return playerState->FindComponentByClass<UCInventoryComponent>();

	return nullptr;
}

void ACWorldItemActor::OnRep_ItemState()
{
	SetActorHiddenInGame(bClaimed);
	SetActorEnableCollision(false == bClaimed);
}
