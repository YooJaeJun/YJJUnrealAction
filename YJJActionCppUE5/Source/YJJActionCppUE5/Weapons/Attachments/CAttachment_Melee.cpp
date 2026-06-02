#include "Weapons/Attachments/CAttachment_Melee.h"

#include "Components/SkeletalMeshComponent.h"

ACAttachment_Fist::ACAttachment_Fist() = default;
ACAttachment_Sword::ACAttachment_Sword() = default;
ACAttachment_Dual::ACAttachment_Dual() = default;

ACAttachment_Hammer::ACAttachment_Hammer() = default;

USkeletalMeshComponent* ACAttachment_Hammer::ResolveSkeletalMesh()
{
	if (CachedSkeletalMesh.IsValid())
	{
		return CachedSkeletalMesh.Get();
	}

	if (UObject* const defaultSubobject = GetDefaultSubobjectByName(TEXT("SkeletalMesh")))
	{
		CachedSkeletalMesh = Cast<USkeletalMeshComponent>(defaultSubobject);
	}

	if (false == CachedSkeletalMesh.IsValid())
	{
		TArray<USkeletalMeshComponent*> meshComponents;
		GetComponents<USkeletalMeshComponent>(meshComponents, true);
		if (meshComponents.Num() > 0)
		{
			CachedSkeletalMesh = meshComponents[0];
		}
	}

	return CachedSkeletalMesh.Get();
}

void ACAttachment_Hammer::SetSkeletalMeshVisible(const bool bVisible)
{
	USkeletalMeshComponent* const skeletalMesh = ResolveSkeletalMesh();
	if (IsValid(skeletalMesh))
	{
		skeletalMesh->SetVisibility(bVisible, false);
	}
}

void ACAttachment_Hammer::BeginPlay()
{
	Super::BeginPlay();

	// 레거시 BP: ReceiveBeginPlay → SkeletalMesh SetVisibility(false)
	SetSkeletalMeshVisible(false);
}

void ACAttachment_Hammer::OnBeginEquip_Implementation()
{
	Super::OnBeginEquip_Implementation();

	// 레거시 BP: OnBeginEquip → Visible(true) → AttachTo("Hand_Hammer")
	SetSkeletalMeshVisible(true);
	AttachTo(HandEquipSocketName);
}

void ACAttachment_Hammer::OnUnequip_Implementation()
{
	Super::OnUnequip_Implementation();

	// 레거시 BP: OnUnequip → SkeletalMesh SetVisibility(false)
	SetSkeletalMeshVisible(false);
}
