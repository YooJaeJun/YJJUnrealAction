#include "Weapons/CWeaponAsset.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CEquipment.h"
#include "Weapons/CAct.h"

UCWeaponAsset::UCWeaponAsset()
{
	AttachmentClass = ACAttachment::StaticClass();
	EquipmentClass = UCEquipment::StaticClass();
	ActClass = UCAct::StaticClass();
}

void UCWeaponAsset::BeginPlay(ACCommonCharacter* InOwner)
{
	if (!!AttachmentClass)
	{
		FActorSpawnParameters params;
		params.Owner = Cast<AActor>(InOwner);

		Attachment = InOwner->GetWorld()->SpawnActor<ACAttachment>(AttachmentClass, params);
	}

	if (!!EquipmentClass)
	{
		Equipment = NewObject<UCEquipment>(this, EquipmentClass);
		Equipment->BeginPlay(InOwner, EquipmentData);

		if (!!Attachment)
		{
			Equipment->OnEquipmentBeginEquip.AddDynamic(Attachment, &ACAttachment::OnBeginEquip);
			Equipment->OnEquipmentUnequip.AddDynamic(Attachment, &ACAttachment::OnUnequip);
		}
	}

	if (!!ActClass)
	{
		Act = NewObject<UCAct>(this, ActClass);
		Act->BeginPlay(InOwner, Attachment, Equipment, ActDatas, HitDatas);

		if (!!Attachment)
		{
			Attachment->OnAttachmentBeginCollision.AddDynamic(Act, &UCAct::OnAttachmentBeginCollision);
			Attachment->OnAttachmentEndCollision.AddDynamic(Act, &UCAct::OnAttachmentEndCollision);
			Attachment->OnAttachmentBeginOverlap.AddDynamic(Act, &UCAct::OnAttachmentBeginOverlap);
			Attachment->OnAttachmentEndOverlap.AddDynamic(Act, &UCAct::OnAttachmentEndOverlap);
		}
	}
}