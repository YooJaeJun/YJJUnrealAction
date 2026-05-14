#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPlayerController.generated.h"

class ACPlacedActor;
class ACWorldItemActor;
class UCInventoryComponent;

UCLASS()
class YJJACTIONCPPUE5_API ACPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACPlayerController();

public:
	UFUNCTION(BlueprintCallable, Category = "Dedicated Server")
	void RequestPickup(ACWorldItemActor* WorldItem);

	UFUNCTION(BlueprintCallable, Category = "Dedicated Server")
	void RequestPlacement(const FTransform& RequestedTransform, const FName ItemID);

protected:
	UFUNCTION(Server, Reliable)
	void Server_RequestPickup(ACWorldItemActor* WorldItem);

	UFUNCTION(Server, Reliable)
	void Server_RequestPlacement(FTransform RequestedTransform, FName ItemID);

	UFUNCTION(Client, Reliable)
	void Client_NotifyServerActionResult(bool bSucceeded, FName Reason);

private:
	bool ValidatePlacementRequest(const FTransform& RequestedTransform, const FName ItemID) const;
	UCInventoryComponent* FindInventoryComponent() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Placement")
	TSubclassOf<ACPlacedActor> DefaultPlacedActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Placement", meta = (ClampMin = "0"))
	float MaxPlacementDistance = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Placement", meta = (ClampMin = "0"))
	float PlacementCollisionRadius = 75.0f;
};
