#include "Weapons/CWeaponAsset.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "CAttachment.h"
#include "CEquipment.h"
#include "CAct.h"

UCWeaponAsset::UCWeaponAsset()
{
	AttachmentClass = ACAttachment::StaticClass();
	EquipmentClass = UCEquipment::StaticClass();
	ActClass = UCAct::StaticClass();
	SkillClass = ACAttachment::StaticClass();
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

	if (!!SkillClass)
	{
		//for (auto& skill : Skills)
		//{
		//	CHelpers::GetAsset<ACAttachment>(&skill, "Blueprint'/Game/Weapons/Bow/BP_CAttachment_Arrow.BP_CAttachment_Arrow_C'");

		//	Equipment->OnEquipmentBeginEquip.AddDynamic(skill, &ACAttachment::OnBeginEquip);
		//	Equipment->OnEquipmentUnequip.AddDynamic(skill, &ACAttachment::OnUnequip);

		//	if (!!Act)
		//	{
		//		skill->OnAttachmentBeginCollision.AddDynamic(Act, &UCAct::OnAttachmentBeginCollision);
		//		skill->OnAttachmentEndCollision.AddDynamic(Act, &UCAct::OnAttachmentEndCollision);

		//		skill->OnAttachmentBeginOverlap.AddDynamic(Act, &UCAct::OnAttachmentBeginOverlap);
		//		skill->OnAttachmentEndOverlap.AddDynamic(Act, &UCAct::OnAttachmentEndOverlap);
		//	}
		//}
	}
}