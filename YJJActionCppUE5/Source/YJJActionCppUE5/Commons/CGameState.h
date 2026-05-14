#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CGameState.generated.h"

UCLASS()
class YJJACTIONCPPUE5_API ACGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ACGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void AddWorldItemCount(const int32 InDelta);
	void AddPlacedActorCount(const int32 InDelta);

	FORCEINLINE int32 GetWorldItemCount() const { return WorldItemCount; }
	FORCEINLINE int32 GetPlacedActorCount() const { return PlacedActorCount; }

private:
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Dedicated Server")
		int32 WorldItemCount = 0;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Dedicated Server")
		int32 PlacedActorCount = 0;
};
