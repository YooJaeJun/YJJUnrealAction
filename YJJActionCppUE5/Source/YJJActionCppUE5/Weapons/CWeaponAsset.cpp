#include "Weapons/CWeaponAsset.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CEquipment.h"
#include "Weapons/CAct.h"
#include "Weapons/CSkill.h"
#include "Weapons/Acts/CAct_Combo.h"
#include "Weapons/Acts/CAct_Bow.h"
#include "Weapons/Acts/CAct_Warp.h"
#include "Weapons/Acts/CAct_Around.h"
#include "Engine/DataTable.h"

namespace WeaponAssetRuntime
{
	struct FWeaponContentPaths
	{
		const TCHAR* FolderName;
		const TCHAR* DoActionTable;
		const TCHAR* HitDataTable;
		const TCHAR* AttachmentBlueprint;
	};

	const FWeaponContentPaths* ResolveContentPaths(const CEWeaponType InType)
	{
		static const FWeaponContentPaths FistPaths{
			TEXT("Fist"),
			TEXT("/Game/Weapons/Fist/DT_DoAction_Fist.DT_DoAction_Fist"),
			TEXT("/Game/Weapons/Fist/DT_HitData_Fist.DT_HitData_Fist"),
			TEXT("/Game/Weapons/Fist/CBP_Attachment_Fist.CBP_Attachment_Fist_C"),
		};
		static const FWeaponContentPaths SwordPaths{
			TEXT("Sword"),
			TEXT("/Game/Weapons/Sword/DT_DoAction_Sword.DT_DoAction_Sword"),
			TEXT("/Game/Weapons/Sword/DT_HitData_Sword.DT_HitData_Sword"),
			TEXT("/Game/Weapons/Sword/CBP_Attachment_Sword.CBP_Attachment_Sword_C"),
		};
		static const FWeaponContentPaths HammerPaths{
			TEXT("Hammer"),
			TEXT("/Game/Weapons/Hammer/DT_DoAction_Hammer.DT_DoAction_Hammer"),
			TEXT("/Game/Weapons/Hammer/DT_HitData_Hammer.DT_HitData_Hammer"),
			TEXT("/Game/Weapons/Hammer/CBP_Attachment_Hammer.CBP_Attachment_Hammer_C"),
		};
		static const FWeaponContentPaths DualPaths{
			TEXT("Dual"),
			TEXT("/Game/Weapons/Dual/DT_DoAction_Dual.DT_DoAction_Dual"),
			TEXT("/Game/Weapons/Dual/DT_HitData_Dual.DT_HitData_Dual"),
			TEXT("/Game/Weapons/Dual/CBP_Attachment_Dual.CBP_Attachment_Dual_C"),
		};
		static const FWeaponContentPaths BowPaths{
			TEXT("Bow"),
			TEXT("/Game/Weapons/Bow/DT_DoAction_Bow.DT_DoAction_Bow"),
			TEXT("/Game/Weapons/Bow/DT_HitData_Bow.DT_HitData_Bow"),
			TEXT("/Game/Weapons/Bow/CBP_Attachment_Bow.CBP_Attachment_Bow_C"),
		};

		switch (InType)
		{
		case CEWeaponType::Fist:
			return &FistPaths;
		case CEWeaponType::Sword:
			return &SwordPaths;
		case CEWeaponType::Hammer:
			return &HammerPaths;
		case CEWeaponType::Dual:
			return &DualPaths;
		case CEWeaponType::Bow:
			return &BowPaths;
		default:
			return nullptr;
		}
	}

	bool IsUsableActClass(const TSubclassOf<UCAct>& InClass)
	{
		return IsValid(InClass) && InClass->IsChildOf(UCAct::StaticClass());
	}

	TSubclassOf<UCAct> ResolveDefaultActClass(const CEWeaponType InType)
	{
		switch (InType)
		{
		case CEWeaponType::Bow:
			return UCAct_Bow::StaticClass();
		case CEWeaponType::Warp:
			return UCAct_Warp::StaticClass();
		case CEWeaponType::Around:
			return UCAct_Around::StaticClass();
		default:
			break;
		}

		const FWeaponContentPaths* const paths = ResolveContentPaths(InType);
		if (nullptr != paths)
		{
			TSubclassOf<UCAct> loadedActClass = nullptr;
			const FString actBpPath = FString::Printf(
				TEXT("/Game/Weapons/%s/CBP_Act_%s.CBP_Act_%s_C"),
				paths->FolderName,
				paths->FolderName,
				paths->FolderName);
			YJJHelpers::GetClassDynamic<UCAct>(&loadedActClass, actBpPath);
			if (IsUsableActClass(loadedActClass))
				return loadedActClass;
		}

		{
			const FString folderName = YJJHelpers::ConvertEnumToString(InType);
			TSubclassOf<UCAct> magicActClass = nullptr;
			const FString actBpPath = FString::Printf(
				TEXT("/Game/Weapons/%s/CBP_Act_%s.CBP_Act_%s_C"),
				*folderName,
				*folderName,
				*folderName);
			YJJHelpers::GetClassDynamic<UCAct>(&magicActClass, actBpPath);
			if (IsUsableActClass(magicActClass))
				return magicActClass;
		}

		return UCAct_Combo::StaticClass();
	}

	void LoadDoActionRowsFromTable(TArray<FDoActionData>& OutRows, const TCHAR* const TablePath)
	{
		if (nullptr == TablePath)
			return;

		UDataTable* const table =
			Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TablePath));
		if (false == IsValid(table))
			return;

		const UScriptStruct* const rowStruct = table->GetRowStruct();
		const UScriptStruct* const expected = FDoActionData::StaticStruct();
		if ((false == IsValid(rowStruct)) || (rowStruct != expected))
			return;

		const TArray<FName> rowNames = table->GetRowNames();
		for (const FName rowName : rowNames)
		{
			const FDoActionData* const row =
				table->FindRow<FDoActionData>(rowName, TEXT("WeaponAsset_DoAction"));
			if (nullptr != row)
				OutRows.Add(*row);
		}
	}

	void LoadHitRowsFromTable(TArray<FHitData>& OutRows, const TCHAR* const TablePath)
	{
		if (nullptr == TablePath)
			return;

		UDataTable* const table =
			Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TablePath));
		if (false == IsValid(table))
			return;

		const UScriptStruct* const rowStruct = table->GetRowStruct();
		const UScriptStruct* const expected = FHitData::StaticStruct();
		if ((false == IsValid(rowStruct)) || (rowStruct != expected))
			return;

		const TArray<FName> rowNames = table->GetRowNames();
		for (const FName rowName : rowNames)
		{
			const FHitData* const row = table->FindRow<FHitData>(rowName, TEXT("WeaponAsset_Hit"));
			if (nullptr != row)
				OutRows.Add(*row);
		}
	}

}

void UCWeaponAsset::EnsureConfigFromContent()
{
	if (false == WeaponAssetRuntime::IsUsableActClass(ActClass))
		ActClass = WeaponAssetRuntime::ResolveDefaultActClass(GetType());

	const WeaponAssetRuntime::FWeaponContentPaths* const paths =
		WeaponAssetRuntime::ResolveContentPaths(GetType());
	if (nullptr == paths)
		return;

	if (false == IsValid(AttachmentClass))
	{
		TSubclassOf<ACAttachment> loadedAttachment = nullptr;
		YJJHelpers::GetClassDynamic<ACAttachment>(&loadedAttachment, paths->AttachmentBlueprint);
		if (IsValid(loadedAttachment))
			AttachmentClass = loadedAttachment;
		else
			AttachmentClass = ACAttachment::StaticClass();
	}

	if (false == IsValid(EquipmentClass))
		EquipmentClass = UCEquipment::StaticClass();

	if (ActDatas.Num() < 1)
		WeaponAssetRuntime::LoadDoActionRowsFromTable(ActDatas, paths->DoActionTable);

	if (HitDatas.Num() < 1)
		WeaponAssetRuntime::LoadHitRowsFromTable(HitDatas, paths->HitDataTable);
}

UCWeaponAsset::UCWeaponAsset()
{
	AttachmentClass = ACAttachment::StaticClass();
	EquipmentClass = UCEquipment::StaticClass();
}

UCWeaponAsset* UCWeaponAsset::CreateRuntimeTemplate(
	UObject* Outer,
	const CEWeaponType InType,
	const CEMagicType InMagicType)
{
	if (false == IsValid(Outer))
		return nullptr;

	UCWeaponAsset* const asset = NewObject<UCWeaponAsset>(Outer, UCWeaponAsset::StaticClass());
	asset->Type = InType;
	asset->MagicType = InMagicType;
	asset->EnsureConfigFromContent();
	return asset;
}

const UCWeaponAsset& UCWeaponAsset::DeepCopy(
	const UCWeaponAsset& InOther,
	const TWeakObjectPtr<ACCommonCharacter> Owner)
{
	Type = InOther.GetType();
	MagicType = InOther.GetMagicType();
	AttachmentClass = InOther.AttachmentClass;
	EquipmentClass = InOther.EquipmentClass;
	EquipmentData = InOther.EquipmentData;
	ActClass = InOther.ActClass;
	ActDatas = InOther.ActDatas;
	HitDatas = InOther.HitDatas;
	SkillClasses = InOther.SkillClasses;

	Attachment = nullptr;
	Equipment = nullptr;
	Act = nullptr;
	Skills.Empty();

	BeginPlay(Owner.Get());

	return *this;
}

void UCWeaponAsset::BeginPlay(TWeakObjectPtr<ACCommonCharacter> InOwner)
{
	if (IsValid(Act))
		return;

	EnsureConfigFromContent();

	if (false == InOwner.IsValid())
		return;

	if (IsValid(Attachment))
	{
		Attachment->Destroy();
		Attachment = nullptr;
	}

	Equipment = nullptr;
	Skills.Empty();

	if (IsValid(AttachmentClass))
	{
		FActorSpawnParameters params;
		params.Owner = Cast<AActor>(InOwner.Get());

		Attachment = InOwner->GetWorld()->SpawnActor<ACAttachment>(AttachmentClass, params);
	}

	if (IsValid(EquipmentClass))
	{
		Equipment = NewObject<UCEquipment>(this, EquipmentClass);
		Equipment->BeginPlay(InOwner, EquipmentData);

		if (IsValid(Attachment))
		{
			Equipment->OnEquipmentBeginEquip.AddDynamic(Attachment, &ACAttachment::OnBeginEquip);
			Equipment->OnEquipmentUnequip.AddDynamic(Attachment, &ACAttachment::OnUnequip);
		}
	}

	if (WeaponAssetRuntime::IsUsableActClass(ActClass))
	{
		Act = NewObject<UCAct>(this, ActClass);
		Act->BeginPlay(InOwner, Attachment, Equipment, ActDatas, HitDatas);

		if (IsValid(Attachment))
		{
			Attachment->OnAttachmentBeginCollision.AddDynamic(Act, &UCAct::OnAttachmentBeginCollision);
			Attachment->OnAttachmentEndCollision.AddDynamic(Act, &UCAct::OnAttachmentEndCollision);
			Attachment->OnAttachmentBeginOverlap.AddDynamic(Act, &UCAct::OnAttachmentBeginOverlap);
			Attachment->OnAttachmentEndOverlap.AddDynamic(Act, &UCAct::OnAttachmentEndOverlap);
		}

		if (IsValid(Equipment))
		{
			Equipment->OnEquipmentBeginEquip.AddDynamic(Act, &UCAct::OnBeginEquip);
			Equipment->OnEquipmentUnequip.AddDynamic(Act, &UCAct::OnUnequip);
		}
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponAsset] UCAct 생성 실패 — Type=%s ActClass=%s ActDatas=%d Owner=%s"),
			*YJJHelpers::ConvertEnumToString(Type),
			IsValid(ActClass) ? *ActClass->GetPathName() : TEXT("(null)"),
			ActDatas.Num(),
			*InOwner->GetName()));
	}

	if (IsValid(Attachment) && IsValid(Act))
	{
		for (int32 skillIndex = 0; skillIndex < SkillClasses.Num(); ++skillIndex)
		{
			if (IsValid(SkillClasses[skillIndex]))
			{
				Skills.Emplace(NewObject<UCSkill>(this, SkillClasses[skillIndex]));
				Skills[skillIndex]->BeginPlay(InOwner, Attachment, Act);
			}
		}
	}
}

UCSkill* UCWeaponAsset::GetSkill(const int32 SkillIndex) const
{
	CheckTrueResult(SkillIndex >= Skills.Num(), nullptr);

	return Skills[SkillIndex];
}
