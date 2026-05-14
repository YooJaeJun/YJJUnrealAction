#include "Commons/CGameState.h"
#include "Net/UnrealNetwork.h"

ACGameState::ACGameState()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACGameState, WorldItemCount);
	DOREPLIFETIME(ACGameState, PlacedActorCount);
}

void ACGameState::AddWorldItemCount(const int32 InDelta)
{
	if (HasAuthority())
		WorldItemCount = FMath::Max(0, WorldItemCount + InDelta);
}

void ACGameState::AddPlacedActorCount(const int32 InDelta)
{
	if (HasAuthority())
		PlacedActorCount = FMath::Max(0, PlacedActorCount + InDelta);
}
