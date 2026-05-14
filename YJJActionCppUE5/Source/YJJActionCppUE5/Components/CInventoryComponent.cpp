#include "Components/CInventoryComponent.h"
#include "Net/UnrealNetwork.h"

bool FCInventoryItemStack::IsValidStack() const
{
	return ItemID != NAME_None && Quantity > 0;
}

UCInventoryComponent::UCInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCInventoryComponent, Items);
}

bool UCInventoryComponent::CanAddItem(const FName InItemID, const int32 InQuantity) const
{
	if (InItemID == NAME_None || InQuantity <= 0)
		return false;

	for (const FCInventoryItemStack& item : Items)
	{
		if (item.ItemID == InItemID && item.Quantity + InQuantity <= DefaultMaxStack)
			return true;
	}

	return Items.Num() < MaxSlots;
}

bool UCInventoryComponent::AddItem(const FName InItemID, const int32 InQuantity)
{
	if (false == GetOwner()->HasAuthority())
		return false;

	if (false == CanAddItem(InItemID, InQuantity))
		return false;

	for (FCInventoryItemStack& item : Items)
	{
		if (item.ItemID == InItemID && item.Quantity + InQuantity <= DefaultMaxStack)
		{
			item.Quantity += InQuantity;
			BroadcastInventoryChanged();

			return true;
		}
	}

	FCInventoryItemStack newItem;
	newItem.ItemID = InItemID;
	newItem.Quantity = InQuantity;
	Items.Add(newItem);

	BroadcastInventoryChanged();

	return true;
}

bool UCInventoryComponent::RemoveItem(const FName InItemID, const int32 InQuantity)
{
	if (false == GetOwner()->HasAuthority())
		return false;

	if (false == HasItem(InItemID, InQuantity))
		return false;

	int32 remainingQuantity = InQuantity;
	for (int32 index = Items.Num() - 1; index >= 0 && remainingQuantity > 0; --index)
	{
		FCInventoryItemStack& item = Items[index];
		if (item.ItemID != InItemID)
			continue;

		const int32 removeQuantity = FMath::Min(item.Quantity, remainingQuantity);
		item.Quantity -= removeQuantity;
		remainingQuantity -= removeQuantity;

		if (item.Quantity <= 0)
			Items.RemoveAt(index);
	}

	BroadcastInventoryChanged();

	return true;
}

bool UCInventoryComponent::HasItem(const FName InItemID, const int32 InQuantity) const
{
	if (InItemID == NAME_None || InQuantity <= 0)
		return false;

	int32 totalQuantity = 0;
	for (const FCInventoryItemStack& item : Items)
	{
		if (item.ItemID == InItemID)
			totalQuantity += item.Quantity;
	}

	return totalQuantity >= InQuantity;
}

void UCInventoryComponent::OnRep_Items()
{
	BroadcastInventoryChanged();
}

void UCInventoryComponent::BroadcastInventoryChanged()
{
	OnInventoryChanged.Broadcast();
}
