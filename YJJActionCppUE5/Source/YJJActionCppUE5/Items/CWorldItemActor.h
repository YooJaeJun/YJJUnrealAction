#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/CInterface_Interactable.h"
#include "CWorldItemActor.generated.h"

class ACCommonCharacter;
class UCInventoryComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class YJJACTIONCPPUE5_API ACWorldItemActor : public AActor, public ICInterface_Interactable
{
	GENERATED_BODY()

public:
	ACWorldItemActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Interact(TObjectPtr<ACCommonCharacter> InteractingActor) override;

public:
	bool TryPickup(AController* RequestController);

	FORCEINLINE FName GetItemID() const { return ItemID; }
	FORCEINLINE int32 GetQuantity() const { return Quantity; }

private:
	bool CanPickupBy(const APawn* RequestPawn) const;
	UCInventoryComponent* FindInventoryComponent(const AController* RequestController) const;

	UFUNCTION()
		void OnRep_ItemState();

protected:
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<USphereComponent> InteractionSphere;

private:
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_ItemState, Category = "Item")
		FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_ItemState, Category = "Item", meta = (ClampMin = "1"))
		int32 Quantity = 1;

	UPROPERTY(ReplicatedUsing = OnRep_ItemState, VisibleAnywhere, Category = "Item")
		bool bClaimed = false;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction", meta = (ClampMin = "0"))
		float PickupDistance = 250.0f;
};
