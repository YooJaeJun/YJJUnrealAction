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
			Equipment->OnEquipmentBeginEquip.AddUniqueDynamic(Attachment, &ACAttachment::OnBeginEquip);
			Equipment->OnEquipmentUnequip.AddUniqueDynamic(Attachment, &ACAttachment::OnUnequip);
		}
	}

	if (!!ActClass)
	{
		Act = NewObject<UCAct>(this, ActClass);
		Act->BeginPlay(InOwner, Attachment, Equipment, ActDatas, HitDatas);

		if (!!Attachment)
		{
			Attachment->OnAttachmentBeginCollision.AddUniqueDynamic(Act, &UCAct::OnAttachmentBeginCollision);
			Attachment->OnAttachmentEndCollision.AddUniqueDynamic(Act, &UCAct::OnAttachmentEndCollision);
			Attachment->OnAttachmentBeginOverlap.AddUniqueDynamic(Act, &UCAct::OnAttachmentBeginOverlap);
			Attachment->OnAttachmentEndOverlap.AddUniqueDynamic(Act, &UCAct::OnAttachmentEndOverlap);
		}
	}
}

const UCWeaponAsset& UCWeaponAsset::DeepCopy(
	const UCWeaponAsset& InOther, 
	const TWeakObjectPtr<ACCommonCharacter> Owner)
{
	Type = InOther.GetType();
	AttachmentClass = InOther.AttachmentClass;
	Attachment = InOther.GetAttachment();
	EquipmentClass = InOther.EquipmentClass;
	EquipmentData = InOther.EquipmentData;
	Equipment = InOther.GetEquipment();
	ActClass = InOther.ActClass;
	ActDatas = InOther.ActDatas;
	Act = InOther.GetAct();
	HitDatas = InOther.HitDatas;

	BeginPlay(Owner.Get());

	return *this;
}
