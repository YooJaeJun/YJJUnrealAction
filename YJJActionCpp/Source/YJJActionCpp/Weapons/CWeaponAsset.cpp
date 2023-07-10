#include "Weapons/CWeaponAsset.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CEquipment.h"
#include "Weapons/CAct.h"
#include "Weapons/CSkill.h"

UCWeaponAsset::UCWeaponAsset()
{
	AttachmentClass = ACAttachment::StaticClass();
	EquipmentClass = UCEquipment::StaticClass();
	// Abstract 클래스는 기본값 초기화 x
	// ActClass = UCAct::StaticClass();
	// SkillClasses = UCSkill::StaticClass();
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
	SkillClasses = InOther.SkillClasses;
	Skills = InOther.Skills;

	BeginPlay(Owner.Get());

	return *this;
}

void UCWeaponAsset::BeginPlay(TWeakObjectPtr<ACCommonCharacter> InOwner)
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

	for (int i=0; i<SkillClasses.Num(); i++)
	{
		if (!!SkillClasses[i] && !!Attachment && !!Act)
		{
			Skills.Emplace(NewObject<UCSkill>(this, SkillClasses[i]));
			Skills[i]->BeginPlay(InOwner, Attachment, Act);
		}
	}
}

UCSkill* UCWeaponAsset::GetSkill(const int32 SkillIndex) const
{
	CheckTrueResult(SkillIndex >= Skills.Num(), nullptr);

	return Skills[SkillIndex];
}