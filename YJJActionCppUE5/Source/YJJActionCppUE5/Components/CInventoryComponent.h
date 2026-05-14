#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventoryChanged);

USTRUCT(BlueprintType)
struct YJJACTIONCPPUE5_API FCInventoryItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Quantity = 0;

	bool IsValidStack() const;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YJJACTIONCPPUE5_API UCInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	bool CanAddItem(const FName InItemID, const int32 InQuantity) const;
	bool AddItem(const FName InItemID, const int32 InQuantity);
	bool RemoveItem(const FName InItemID, const int32 InQuantity);
	bool HasItem(const FName InItemID, const int32 InQuantity) const;

	FORCEINLINE const TArray<FCInventoryItemStack>& GetItems() const { return Items; }

private:
	UFUNCTION()
	void OnRep_Items();

	void BroadcastInventoryChanged();

public:
	UPROPERTY(BlueprintAssignable)
	FInventoryChanged OnInventoryChanged;

private:
	UPROPERTY(ReplicatedUsing = OnRep_Items, VisibleAnywhere, Category = "Inventory")
	TArray<FCInventoryItemStack> Items;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 MaxSlots = 32;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 DefaultMaxStack = 99;
};
