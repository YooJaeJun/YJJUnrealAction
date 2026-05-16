#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPlayerController.generated.h"

class ACPlacedActor;
class ACWorldItemActor;
class UCUserWidget_HUD;
class UCInventoryComponent;

UCLASS()
class YJJACTIONCPPUE5_API ACPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACPlayerController();

	virtual void BeginPlay() override;
	virtual void AcknowledgePossession(APawn* P) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Dedicated Server")
	void RequestPickup(ACWorldItemActor* WorldItem);

	UFUNCTION(BlueprintCallable, Category = "Dedicated Server")
	void RequestPlacement(const FTransform& RequestedTransform, const FName ItemID);

	UFUNCTION(BlueprintCallable, Category = "UI")
	UCUserWidget_HUD* EnsureHUD();

	UFUNCTION(BlueprintPure, Category = "UI")
	UCUserWidget_HUD* GetYJJHUD() const;

	// 구 BP_PlayerController 의 GetHUD 커스텀 출력 핀(WBHUDUI) 대체 — EnsureHUD 로 생성까지 맞춘다.
	UFUNCTION(BlueprintCallable, Category = "UI", meta = (DisplayName = "Get WB HUDUI"))
	UCUserWidget_HUD* GetWBHUDUI();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeHUDForPawn(APawn* InPawn);

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
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCUserWidget_HUD> PlayerHUDClass;

	UPROPERTY(Transient)
	TObjectPtr<UCUserWidget_HUD> PlayerHUD;

	UPROPERTY(EditDefaultsOnly, Category = "Placement")
	TSubclassOf<ACPlacedActor> DefaultPlacedActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Placement", meta = (ClampMin = "0"))
	float MaxPlacementDistance = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Placement", meta = (ClampMin = "0"))
	float PlacementCollisionRadius = 75.0f;
};
