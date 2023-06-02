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

				Equipment->OnEquipmentBeginEquip.AddDynamic(skill, &ACAttachment::OnBeginEquip);
				Equipment->OnEquipmentUnequip.AddDynamic(skill, &ACAttachment::OnUnequip);

				if (!!Act)
				{
					skill->OnAttachmentBeginCollision.AddDynamic(Act, &UCAct::OnAttachmentBeginCollision);
					skill->OnAttachmentEndCollision.AddDynamic(Act, &UCAct::OnAttachmentEndCollision);
					skill->OnAttachmentBeginOverlap.AddDynamic(Act, &UCAct::OnAttachmentBeginOverlap);
					skill->OnAttachmentEndOverlap.AddDynamic(Act, &UCAct::OnAttachmentEndOverlap);
				}
			}
		}
	}
}
