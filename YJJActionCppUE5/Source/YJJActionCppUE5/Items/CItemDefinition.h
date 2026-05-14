#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CItemDefinition.generated.h"

class ACPlacedActor;
class ACWorldItemActor;

UCLASS(BlueprintType)
class YJJACTIONCPPUE5_API UCItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemID = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
	int32 MaxStack = 99;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "World")
	TSubclassOf<ACWorldItemActor> WorldItemClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement")
	TSubclassOf<ACPlacedActor> PlacementActorClass;
};
