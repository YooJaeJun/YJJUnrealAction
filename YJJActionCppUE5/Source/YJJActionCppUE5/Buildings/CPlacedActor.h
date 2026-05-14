#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPlacedActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class YJJACTIONCPPUE5_API ACPlacedActor : public AActor
{
	GENERATED_BODY()

public:
	ACPlacedActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void InitializePlacedActor(const FName InItemID);
	void EnterPlacedDormancy();

	FORCEINLINE FName GetItemID() const { return ItemID; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComp;

private:
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Placement")
	FName ItemID = NAME_None;
};
