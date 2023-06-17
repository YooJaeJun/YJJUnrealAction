#include "Weapons/DataAssets/CWeaponAsset_Bow.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Utilities/CHelpers.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CEquipment.h"
#include "Weapons/CAct.h"

UCWeaponAsset_Bow::UCWeaponAsset_Bow()
{
	SkillClass = ACAttachment::StaticClass();
}

void UCWeaponAsset_Bow::BeginPlay(ACCommonCharacter* InOwner)
{
	Super::BeginPlay(InOwner);

	if (!!SkillClass)
	{
		FActorSpawnParameters params;
		params.Owner = Cast<AActor>(InOwner);

		for (int i = 0; i < SkillMax; i++)
			Skills.Push(InOwner->GetWorld()->SpawnActor<ACAttachment>(SkillClass, params));

		if (!!Equipment)
		{
			for (auto& skill : Skills)
			{
				CheckNull(skill);

				Equipment->OnEquipmentBeginEquip.AddUniqueDynamic(skill, &ACAttachment::OnBeginEquip);
				Equipment->OnEquipmentUnequip.AddUniqueDynamic(skill, &ACAttachment::OnUnequip);

				if (!!Act)
				{
					skill->OnAttachmentBeginCollision.AddUniqueDynamic(Act, &UCAct::OnAttachmentBeginCollision);
					skill->OnAttachmentEndCollision.AddUniqueDynamic(Act, &UCAct::OnAttachmentEndCollision);
					skill->OnAttachmentBeginOverlap.AddUniqueDynamic(Act, &UCAct::OnAttachmentBeginOverlap);
					skill->OnAttachmentEndOverlap.AddUniqueDynamic(Act, &UCAct::OnAttachmentEndOverlap);
				}
			}
		}
	}
}
