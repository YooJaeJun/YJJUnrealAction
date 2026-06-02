#include "Components/CWeaponComponent.h"
#include "Global.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CMagicComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CEquipment.h"
#include "Weapons/CAct.h"
#include "Weapons/CSkill.h"
#include "Weapons/CWeaponAsset.h"
#include "Weapons/CSkillWeapon.h"
#include "Weapons/CWeapon.h"
#include "Weapons/CWeaponCombo.h"
#include "Weapons/CWeaponComboVisual.h"
#include "Weapons/ICombatActionHost.h"
#include "Weapons/Acts/CAct_Combo.h"
#include "Weapons/Bow/CWeaponBow.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UnrealType.h"
#include "UObject/EnumProperty.h"

namespace
{
	/** CDA uasset 는 레거시 CWeaponAsset 부모를 가질 수 있어 클래스 필터 StaticLoadObject 보다 TryLoad·무타입 로드가 리다이렉트에 유리하다. */
	static UCWeaponAsset* TryLoadWeaponDataAssetFromContentPath(const FString& AssetPath)
	{
		if (AssetPath.IsEmpty())
		{
			return nullptr;
		}

		const FSoftObjectPath softPath(AssetPath);
		UObject* loadedObject = softPath.TryLoad();
		if (IsValid(loadedObject))
		{
			UCWeaponAsset* const asWeaponAsset = Cast<UCWeaponAsset>(loadedObject);
			if (IsValid(asWeaponAsset))
			{
				return asWeaponAsset;
			}

			CLog::Log(FString::Printf(
				TEXT("[Weapon] CDA 클래스 불일치 — %s Stored=%s (Reparent=UCWeaponAsset)"),
				*AssetPath,
				*loadedObject->GetClass()->GetPathName()));
			return nullptr;
		}

		// TryLoad 실패 시 무타입 1회만 — typed StaticLoadObject 는 실패할 때 LogUObjectGlobals 6줄 스팸.
		loadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);
		if (IsValid(loadedObject))
		{
			UCWeaponAsset* const asWeaponAsset = Cast<UCWeaponAsset>(loadedObject);
			if (IsValid(asWeaponAsset))
			{
				return asWeaponAsset;
			}

			CLog::Log(FString::Printf(
				TEXT("[Weapon] CDA 클래스 불일치 — %s Stored=%s (Reparent=UCWeaponAsset)"),
				*AssetPath,
				*loadedObject->GetClass()->GetPathName()));
		}

		return nullptr;
	}
	bool Legacy_InputParmIsEligible(const FProperty* const Prop)
	{
		CheckNullResult(Prop, false);
		if ((Prop->PropertyFlags & CPF_Parm) == 0)
			return false;
		if (Prop->HasAnyPropertyFlags(CPF_ReturnParm))
			return false;
		if (Prop->HasAnyPropertyFlags(CPF_OutParm))
			return false;
		return true;
	}

	// 레거시 BP 의 DoAction(Enum, int) 패킹이 에디터별로 미세하게 달라 직접 struct 를 쓰기 어렵다 — Parms 버퍼에 반영 속성 순서대로 채운다.
	bool TryDispatch_ProcessEvent_WithAttackSkillIndexParms(
		UObject* const TargetActor,
		const FName FunctionName,
		const CEAttackType InAttackType,
		const int32 InSkillIndex)
	{
		CheckNullResult(TargetActor, false);

		UFunction* const Fn = TargetActor->FindFunction(FunctionName);
		if (nullptr == Fn)
			return false;

		const int32 ParmsByteSize = Fn->ParmsSize;
		if (ParmsByteSize <= 0)
		{
			TargetActor->ProcessEvent(Fn, nullptr);
			return true;
		}

		TArray<uint8> ParamsScratch;
		ParamsScratch.SetNumZeroed(ParmsByteSize);
		uint8* const ParmsMem = ParamsScratch.GetData();

		bool bWroteKnownField = false;
		bool bSawEligibleCategoryField = false;
		bool bSawForeignInputParm = false;

		bool bEnumSlotFilled = false;
		bool bIntSlotFilled = false;

		for (TFieldIterator<FProperty> Iterator(Fn, EFieldIteratorFlags::ExcludeSuper); Iterator; ++Iterator)
		{
			FProperty* Property = *Iterator;
			if (false == Legacy_InputParmIsEligible(Property))
				continue;

			if (nullptr != CastField<FBoolProperty>(Property))
			{
				bSawForeignInputParm = true;
				continue;
			}

			const FEnumProperty* const AsEnumProp = CastField<FEnumProperty>(Property);
			const FByteProperty* const AsByteProp = CastField<FByteProperty>(Property);
			const FIntProperty* const AsIntProp = CastField<FIntProperty>(Property);

			if ((nullptr == AsEnumProp) && (nullptr == AsByteProp) && (nullptr == AsIntProp))
			{
				bSawForeignInputParm = true;
				continue;
			}

			bSawEligibleCategoryField = true;

			if ((nullptr != AsEnumProp) && (false == bEnumSlotFilled))
			{
				FNumericProperty* const Under = AsEnumProp->GetUnderlyingProperty();
				CheckNullResult(Under, false);
				void* const Addr = Property->ContainerPtrToValuePtr<void>(ParmsMem);
				Under->SetIntPropertyValue(Addr, static_cast<int64>(static_cast<uint8>(InAttackType)));
				bEnumSlotFilled = true;
				bWroteKnownField = true;
				continue;
			}

			if ((nullptr != AsByteProp) && AsByteProp->IsEnum() && (false == bEnumSlotFilled))
			{
				void* const Addr = Property->ContainerPtrToValuePtr<void>(ParmsMem);
				AsByteProp->SetIntPropertyValue(Addr, static_cast<int64>(static_cast<uint8>(InAttackType)));
				bEnumSlotFilled = true;
				bWroteKnownField = true;
				continue;
			}

			if ((nullptr != AsIntProp) && (false == bIntSlotFilled))
			{
				void* const Addr = Property->ContainerPtrToValuePtr<void>(ParmsMem);
				AsIntProp->SetIntPropertyValue(Addr, static_cast<int64>(InSkillIndex));
				bIntSlotFilled = true;
				bWroteKnownField = true;
				continue;
			}
		}

		if (bSawForeignInputParm)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[CWeaponComponent][TryDispatch_ProcessEvent_WithAttackSkillIndexParms] 미지원 인자 타입이라 호출 포기(Target=%s, Fn=%s)"),
				*TargetActor->GetName(),
				*FunctionName.ToString());

			return false;
		}

		if ((false == bWroteKnownField) && (true == bSawEligibleCategoryField))
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[CWeaponComponent][TryDispatch_ProcessEvent_WithAttackSkillIndexParms] 인자 패턴 인식 불가(Target=%s, Fn=%s)"),
				*TargetActor->GetName(),
				*FunctionName.ToString());

			return false;
		}

		TargetActor->ProcessEvent(Fn, ParmsMem);
		return true;
	}

	bool TryDispatch_ProcessEvent_AttackEnumOnlyParms_WithValue(UObject* const TargetActor, const FName FunctionName,
	                                                            const CEAttackType InAttackType)
	{
		CheckNullResult(TargetActor, false);

		UFunction* const Fn = TargetActor->FindFunction(FunctionName);
		if (nullptr == Fn)
			return false;

		const int32 ParmsByteSize = Fn->ParmsSize;
		if (ParmsByteSize <= 0)
		{
			TargetActor->ProcessEvent(Fn, nullptr);
			return true;
		}

		TArray<uint8> ParamsScratch;
		ParamsScratch.SetNumZeroed(ParmsByteSize);
		uint8* const ParmsMem = ParamsScratch.GetData();

		bool bWroteKnownField = false;
		bool bSawEligibleCategoryField = false;
		bool bSawForeignInputParm = false;

		for (TFieldIterator<FProperty> Iterator(Fn, EFieldIteratorFlags::ExcludeSuper); Iterator; ++Iterator)
		{
			FProperty* Property = *Iterator;
			if (false == Legacy_InputParmIsEligible(Property))
				continue;

			if (nullptr != CastField<FBoolProperty>(Property))
			{
				bSawForeignInputParm = true;
				continue;
			}

			const FEnumProperty* const AsEnumProp = CastField<FEnumProperty>(Property);
			const FByteProperty* const AsByteProp = CastField<FByteProperty>(Property);

			if ((nullptr == AsEnumProp) && ((nullptr == AsByteProp) || (false == AsByteProp->IsEnum())))
			{
				bSawForeignInputParm = true;
				continue;
			}

			bSawEligibleCategoryField = true;

			if (nullptr != AsEnumProp)
			{
				FNumericProperty* const Under = AsEnumProp->GetUnderlyingProperty();
				CheckNullResult(Under, false);
				void* const Addr = Property->ContainerPtrToValuePtr<void>(ParmsMem);
				Under->SetIntPropertyValue(Addr, static_cast<int64>(static_cast<uint8>(InAttackType)));
				bWroteKnownField = true;
				break;
			}

			if (nullptr != AsByteProp)
			{
				void* const Addr = Property->ContainerPtrToValuePtr<void>(ParmsMem);
				AsByteProp->SetIntPropertyValue(Addr, static_cast<int64>(static_cast<uint8>(InAttackType)));
				bWroteKnownField = true;
				break;
			}
		}

		if (bSawForeignInputParm)
			return false;
		if ((false == bWroteKnownField) && (true == bSawEligibleCategoryField))
			return false;

		TargetActor->ProcessEvent(Fn, ParmsMem);
		return true;
	}

	bool TryDispatch_ProcessEvent_FirstInt32Parm(UObject* const TargetActor, const FName FunctionName,
	                                             const int32 InValue)
	{
		CheckNullResult(TargetActor, false);

		UFunction* const Fn = TargetActor->FindFunction(FunctionName);
		if (nullptr == Fn)
			return false;

		const int32 ParmsByteSize = Fn->ParmsSize;
		if (ParmsByteSize <= 0)
		{
			TargetActor->ProcessEvent(Fn, nullptr);
			return true;
		}

		TArray<uint8> ParamsScratch;
		ParamsScratch.SetNumZeroed(ParmsByteSize);
		uint8* const ParmsMem = ParamsScratch.GetData();

		bool bWroteKnownField = false;
		bool bSawEligibleCategoryField = false;
		bool bSawForeignInputParm = false;

		for (TFieldIterator<FProperty> Iterator(Fn, EFieldIteratorFlags::ExcludeSuper); Iterator; ++Iterator)
		{
			FProperty* Property = *Iterator;
			if (false == Legacy_InputParmIsEligible(Property))
				continue;

			if (nullptr != CastField<FBoolProperty>(Property))
			{
				bSawForeignInputParm = true;
				continue;
			}

			const FIntProperty* const AsIntProp = CastField<FIntProperty>(Property);
			if (nullptr == AsIntProp)
			{
				bSawForeignInputParm = true;
				continue;
			}

			bSawEligibleCategoryField = true;
			void* const Addr = Property->ContainerPtrToValuePtr<void>(ParmsMem);
			AsIntProp->SetIntPropertyValue(Addr, static_cast<int64>(InValue));
			bWroteKnownField = true;
			break;
		}

		if (bSawForeignInputParm)
			return false;
		if ((false == bWroteKnownField) && (true == bSawEligibleCategoryField))
			return false;

		TargetActor->ProcessEvent(Fn, ParmsMem);
		return true;
	}

	bool TryDispatch_ProcessEvent_FirstBoolParm(UObject* const TargetActor, const FName FunctionName, const bool bValue)
	{
		CheckNullResult(TargetActor, false);

		UFunction* const Fn = TargetActor->FindFunction(FunctionName);
		if (nullptr == Fn)
			return false;

		const int32 ParmsByteSize = Fn->ParmsSize;
		if (ParmsByteSize <= 0)
		{
			TargetActor->ProcessEvent(Fn, nullptr);
			return true;
		}

		TArray<uint8> ParamsScratch;
		ParamsScratch.SetNumZeroed(ParmsByteSize);
		uint8* const ParmsMem = ParamsScratch.GetData();

		bool bWroteKnownField = false;

		for (TFieldIterator<FProperty> Iterator(Fn, EFieldIteratorFlags::ExcludeSuper); Iterator; ++Iterator)
		{
			FProperty* Property = *Iterator;
			if (false == Legacy_InputParmIsEligible(Property))
				continue;

			const FBoolProperty* const BoolProp = CastField<FBoolProperty>(Property);
			if (nullptr == BoolProp)
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[CWeaponComponent][TryDispatch_ProcessEvent_FirstBoolParm] bool 이 아닌 인자 존재 — 레거시 Equip 패킹 불명(Target=%s, Fn=%s)"),
					*TargetActor->GetName(),
					*FunctionName.ToString());

				return false;
			}

			void* const Addr = Property->ContainerPtrToValuePtr<void>(ParmsMem);
			BoolProp->SetPropertyValue(Addr, bValue);
			bWroteKnownField = true;
			break;
		}

		if (false == bWroteKnownField)
			return false;

		TargetActor->ProcessEvent(Fn, ParmsMem);
		return true;
	}

	void TryLegacyWeaponProcessEventIfBound(AActor* const WeaponActor, const FName& FunctionName);

	// 레거시 BP 무기 레인 구분 플래그 — Begin_Equip / End_Unequip 등에 bool 이 있으면 사용, 없으면 무 인자 노드만 호출한다.
	void DispatchLegacyEquipOrBeginEquipGate(AActor* const WeaponActor, const bool bMainLaneWeaponGate)
	{
		if (false == IsValid(WeaponActor))
			return;

		static const FName Begin(TEXT("Begin_Equip"));

		if (TryDispatch_ProcessEvent_FirstBoolParm(WeaponActor, Begin, bMainLaneWeaponGate))
			return;

		static const FName Equip(TEXT("Equip"));
		TryLegacyWeaponProcessEventIfBound(WeaponActor, Equip);
	}

	void DispatchLegacyUnequipOrEndUnequipGate(AActor* const WeaponActor, const bool bMainLaneWeaponGate)
	{
		if (false == IsValid(WeaponActor))
			return;

		static const FName EndUnequip(TEXT("End_Unequip"));

		if (TryDispatch_ProcessEvent_FirstBoolParm(WeaponActor, EndUnequip, bMainLaneWeaponGate))
			return;

		static const FName Unequip(TEXT("Unequip"));
		TryLegacyWeaponProcessEventIfBound(WeaponActor, Unequip);
	}

	// 레거시 BP 무기 레인 플래그 — End_Equip 에 bool 이 있으면 사용하고, 없으면 무인자 호출만 시도한다(Equip 폴백은 없음: 착장 해제와 혼동).
	void DispatchLegacyEndEquipGate(AActor* const WeaponActor, const bool bMainLaneWeaponGate)
	{
		if (false == IsValid(WeaponActor))
			return;

		static const FName EndEquip(TEXT("End_Equip"));

		if (TryDispatch_ProcessEvent_FirstBoolParm(WeaponActor, EndEquip, bMainLaneWeaponGate))
			return;

		TryLegacyWeaponProcessEventIfBound(WeaponActor, EndEquip);
	}

	// 레거시 BP — Begin_Unequip bool 게이트가 있으면 사용, 없으면 무인자 Begin_Unequip 만 시도한다(Unequip 폴백 금지: 타이밍이 다름).
	void DispatchLegacyBeginUnequipGate(AActor* const WeaponActor, const bool bMainLaneWeaponGate)
	{
		if (false == IsValid(WeaponActor))
			return;

		static const FName BeginUnequip(TEXT("Begin_Unequip"));

		if (TryDispatch_ProcessEvent_FirstBoolParm(WeaponActor, BeginUnequip, bMainLaneWeaponGate))
			return;

		TryLegacyWeaponProcessEventIfBound(WeaponActor, BeginUnequip);
	}

	// 레거시 BP Weapon 의 커스텀 이벤트 Equip/Unequip — 네이티브 액터는 FindFunction 이 실패할 수 있다(VERBOSE 로만 통지).
	void TryLegacyWeaponProcessEventIfBound(AActor* const WeaponActor, const FName& FunctionName)
	{
		if (false == IsValid(WeaponActor))
			return;

		UFunction* const Fn = WeaponActor->FindFunction(FunctionName);
		if (Fn != nullptr)
		{
			WeaponActor->ProcessEvent(Fn, nullptr);
			return;
		}

		UE_LOG(LogTemp, Verbose, TEXT("[CWeaponComponent] %s 에 \'%s\' 없음(WBP Weapon_C 규약 또는 네이티브 전환 중)."),
			*WeaponActor->GetName(),
			*FunctionName.ToString());
	}

	/** MainWeapon 이 ACWeaponCombo/RandomPattern 이면 ICombatActionHost 로 콜리전을 켠다. */
	bool TryDispatchWeaponCollisionHost(ACWeapon* const Weapon, const bool bCollisionOn)
	{
		if (false == IsValid(Weapon))
		{
			return false;
		}

		if (false == (Cast<ACWeaponCombo>(Weapon) || Cast<ACWeaponRandomPattern>(Weapon)))
		{
			return false;
		}

		ICombatActionHost* const Host = Cast<ICombatActionHost>(Weapon);
		if (nullptr == Host)
		{
			return false;
		}

		if (bCollisionOn)
		{
			Host->Host_OnCollisions();
		}
		else
		{
			Host->Host_OffCollisions();
		}

		return true;
	}

	bool TrySpawnOrReplaceEquippedWeapon(
		UWorld* World,
		TObjectPtr<ACWeapon>& SlotWeapon,
		const TSubclassOf<ACWeapon>& WeaponClass,
		AActor* OwnerActor)
	{
		CheckNullResult(World, false);
		CheckNullResult(OwnerActor, false);

		if (false == UKismetSystemLibrary::IsValidClass(WeaponClass))
		{
			// 레거시 BP와 동일: 클래스가 무효면 해당 슬롯은 건드리지 않는다(잘못된 인덱스/에디터 초기값).
			return false;
		}

		if (IsValid(SlotWeapon))
		{
			SlotWeapon->Destroy();
			SlotWeapon = nullptr;
		}

		FActorSpawnParameters Params;
		Params.Owner = OwnerActor;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;

		ACWeapon* const SpawnedWeapon = World->SpawnActor<ACWeapon>(WeaponClass, FTransform::Identity, Params);

		if (false == IsValid(SpawnedWeapon))
		{
			UE_LOG(LogTemp, Warning, TEXT("[CWeaponComponent] 무기 액터 스폰 실패(Class=%s, Owner=%s)"),
				*WeaponClass->GetPathName(),
				IsValid(OwnerActor) ? *OwnerActor->GetPathName() : TEXT("<null Owner>"));

			return false;
		}

		SlotWeapon = SpawnedWeapon;
		return true;
	}

	bool TrySpawnOrReplaceEquippedActor(UWorld* World, TObjectPtr<AActor>& SlotActor, const TSubclassOf<AActor>& ActorClass,
	                                   AActor* OwnerActor)
	{
		CheckNullResult(World, false);
		CheckNullResult(OwnerActor, false);

		if (false == UKismetSystemLibrary::IsValidClass(ActorClass))
		{
			// 레거시 BP와 동일: 클래스가 무효면 해당 슬롯은 건드리지 않는다(잘못된 인덱스/에디터 초기값).
			return false;
		}

		if (IsValid(SlotActor))
		{
			SlotActor->Destroy();
			SlotActor = nullptr;
		}

		FActorSpawnParameters Params;
		Params.Owner = OwnerActor;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;

		AActor* const SpawnedActor = World->SpawnActor<AActor>(ActorClass, FTransform::Identity, Params);

		if (false == IsValid(SpawnedActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("[CWeaponComponent] 무기·아머 액터 스폰 실패(Class=%s, Owner=%s)"),
				*ActorClass->GetPathName(),
				IsValid(OwnerActor) ? *OwnerActor->GetPathName() : TEXT("<null Owner>"));

			return false;
		}

		SlotActor = SpawnedActor;
		return true;
	}

	struct FMainWeaponSlotDefault
	{
		const TCHAR* ContentPath;
		UClass* NativeClass;
	};

	TSubclassOf<ACWeapon> ResolveMainWeaponClassForSlot(
		const FMainWeaponSlotDefault& SlotDefault)
	{
		TSubclassOf<ACWeapon> resolvedClass = nullptr;
		if (nullptr != SlotDefault.ContentPath)
		{
			YJJHelpers::GetClassDynamic<ACWeapon>(&resolvedClass, SlotDefault.ContentPath);
		}

		if (false == IsValid(resolvedClass) && IsValid(SlotDefault.NativeClass))
		{
			resolvedClass = SlotDefault.NativeClass;
		}

		return resolvedClass;
	}
} // namespace

UCWeaponComponent::UCWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACCommonCharacter>(GetOwner());

	// BP 기본 9 슬롯 — 없으면 런타임에서 인덱스 접근 시 BP 호환 슬롯 수와 어긋난다.
	MainWeapons.SetNumZeroed(WeaponEquipmentSlotCount);
	SubWeapons.SetNumZeroed(WeaponEquipmentSlotCount);
}

void UCWeaponComponent::DispatchEquippedMagicDelegates(CEMagicType CurrentMagic, CEMagicType PreviousMagic)
{
	AActor* own = GetOwner();
	UCMagicComponent* magicDelegator = nullptr;
	if (IsValid(own))
		magicDelegator = Cast<UCMagicComponent>(own->GetComponentByClass(UCMagicComponent::StaticClass()));

	if (IsValid(magicDelegator))
		magicDelegator->SyncEquippedMagicAfterWeaponPipeline(CurrentMagic, PreviousMagic);
	else if (OnMagicTypeChanged.IsBound())
		OnMagicTypeChanged.Broadcast(CurrentMagic, PreviousMagic);
}

void UCWeaponComponent::SyncBpWeaponLanes()
{
	if (bMagicEquipped)
		return;

	if (MainType == CEWeaponType::Unarmed)
		return;

	if (PhysicalType == MainType)
		return;

	// PhysicalType 만 Unarmed 인데 MainType 은 무장 — ChangeBlueprintWeaponLanes 직후 LMB 무반응의 주 원인.
	if (PhysicalType != CEWeaponType::Unarmed)
		return;

	const TObjectPtr<UCWeaponAsset>* foundAsset = WeaponAssetMap.Find(MainType);
	if (nullptr == foundAsset)
		return;

	ChangePhysicalType(MainType);
}

void UCWeaponComponent::RefreshOwnerCache()
{
	ACCommonCharacter* const ownerChar = Cast<ACCommonCharacter>(GetOwner());
	Owner = ownerChar;

	if (IsValid(ownerChar))
	{
		StateComp = YJJHelpers::GetComponent<UCStateComponent>(ownerChar);
		MovementComp = YJJHelpers::GetComponent<UCMovementComponent>(ownerChar);
	}
}

void UCWeaponComponent::MergeDataAssetsFromSiblingComponents()
{
	AActor* const ownerActor = GetOwner();
	if (false == IsValid(ownerActor))
		return;

	TArray<UCWeaponComponent*> weaponComponents;
	ownerActor->GetComponents<UCWeaponComponent>(weaponComponents);

	const int32 componentCount = weaponComponents.Num();
	for (int32 componentIndex = 0; componentIndex < componentCount; ++componentIndex)
	{
		UCWeaponComponent* const otherComp = weaponComponents[componentIndex];
		if ((false == IsValid(otherComp)) || (otherComp == this))
			continue;

		for (const TObjectPtr<UCWeaponAsset>& sourceAsset : otherComp->DataAssets)
		{
			if (IsValid(sourceAsset))
				DataAssets.AddUnique(sourceAsset);
		}
	}
}

void UCWeaponComponent::CreateSyntheticDataAssetsIfStillEmpty()
{
	if (DataAssets.Num() > 0)
		return;

	struct FSyntheticWeaponEntry
	{
		CEWeaponType WeaponType;
		CEMagicType MagicType;
	};

	static const FSyntheticWeaponEntry SyntheticEntries[] = {
		{ CEWeaponType::Fist, CEMagicType::Unarmed },
		{ CEWeaponType::Sword, CEMagicType::Unarmed },
		{ CEWeaponType::Hammer, CEMagicType::Unarmed },
		{ CEWeaponType::Dual, CEMagicType::Unarmed },
		{ CEWeaponType::Bow, CEMagicType::Unarmed },
		{ CEWeaponType::Warp, CEMagicType::Warp },
		{ CEWeaponType::Around, CEMagicType::Around },
		{ CEWeaponType::Fireball, CEMagicType::FireBall },
		{ CEWeaponType::Bomb, CEMagicType::Bomb },
		{ CEWeaponType::Yondu, CEMagicType::Yondu },
	};

	UObject* const outer = IsValid(GetOwner()) ? GetOwner() : static_cast<UObject*>(this);
	int32 createdCount = 0;

	for (const FSyntheticWeaponEntry& entry : SyntheticEntries)
	{
		UCWeaponAsset* const templateAsset =
			UCWeaponAsset::CreateRuntimeTemplate(outer, entry.WeaponType, entry.MagicType);
		if (IsValid(templateAsset))
		{
			DataAssets.Add(templateAsset);
			++createdCount;
		}
	}

	if (createdCount > 0)
	{
		CLog::LogDisplay(FString::Printf(
			TEXT("[Weapon] CDA Content 로드 실패 — 런타임 합성 DA %d개 사용 Comp=%s Owner=%s"),
			createdCount,
			*GetName(),
			IsValid(GetOwner()) ? *GetOwner()->GetName() : TEXT("(null)")));
	}
}

void UCWeaponComponent::EnsureWeaponPipelineReady()
{
	RefreshOwnerCache();

	if (false == Owner.IsValid())
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] EnsureWeaponPipelineReady: Owner 캐시 없음 — Comp=%s Actor=%s"),
			*GetName(),
			IsValid(GetOwner()) ? *GetOwner()->GetName() : TEXT("(null)")));
		return;
	}

	MergeDataAssetsFromSiblingComponents();
	EnsureDataAssets();

	if (WeaponAssetMap.Num() < 1 && MagicAssetMap.Num() < 1)
		RebuildAssetMaps();
}

void UCWeaponComponent::LogWeaponPipelineStatus(const TCHAR* Context)
{
	const AActor* const ownerActor = GetOwner();
	const UCAct* const act = GetAct();
	CLog::LogDisplay(FString::Printf(
		TEXT("[Weapon][%s] Comp=%s Owner=%s OwnerCache=%s DataAssets=%d Map=%d MagicMap=%d Physical=%s Main=%s Act=%s Equip=%s"),
		Context,
		*GetName(),
		IsValid(ownerActor) ? *ownerActor->GetName() : TEXT("(null)"),
		Owner.IsValid() ? TEXT("O") : TEXT("X"),
		DataAssets.Num(),
		WeaponAssetMap.Num(),
		MagicAssetMap.Num(),
		*YJJHelpers::ConvertEnumToString(PhysicalType),
		*YJJHelpers::ConvertEnumToString(MainType),
		IsValid(act) ? TEXT("O") : TEXT("X"),
		IsValid(GetEquipment()) ? TEXT("O") : TEXT("X")));
}

void UCWeaponComponent::EnsureCombatWeaponEquipped()
{
	if (bMagicEquipped)
		return;

	if (PhysicalType != CEWeaponType::Unarmed)
		return;

	CEWeaponType desiredType = MainType;
	if (desiredType == CEWeaponType::Unarmed)
		desiredType = CEWeaponType::Fist;

	if (false == WeaponAssetMap.Contains(desiredType))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] EnsureCombatWeaponEquipped — Map에 %s 없음(Map=%d DataAssets=%d) Comp=%s"),
			*YJJHelpers::ConvertEnumToString(desiredType),
			WeaponAssetMap.Num(),
			DataAssets.Num(),
			*GetName()));
		return;
	}

	SetMode(desiredType);
}

CEWeaponType UCWeaponComponent::ResolveCombatWeaponLookupType() const
{
	if (bMagicEquipped)
		return CEWeaponType::Unarmed;

	if (PhysicalType != CEWeaponType::Unarmed)
		return PhysicalType;

	if (MainType != CEWeaponType::Unarmed)
		return MainType;

	return CEWeaponType::Unarmed;
}

void UCWeaponComponent::EnsureDataAssets()
{
	MergeDataAssetsFromSiblingComponents();

	if (DataAssets.Num() > 0)
		return;

	AActor* const ownerActor = GetOwner();

	static const TCHAR* const DefaultWeaponDataPaths[] = {
		TEXT("/Game/Weapons/Fist/CDA_Fist.CDA_Fist"),
		TEXT("/Game/Weapons/Sword/CDA_Sword.CDA_Sword"),
		TEXT("/Game/Weapons/Hammer/CDA_Hammer.CDA_Hammer"),
		TEXT("/Game/Weapons/Dual/CDA_Dual.CDA_Dual"),
		TEXT("/Game/Weapons/Bow/CDA_Bow.CDA_Bow"),
		TEXT("/Game/Weapons/Warp/CDA_Warp.CDA_Warp"),
		TEXT("/Game/Weapons/Around/CDA_Around.CDA_Around"),
		TEXT("/Game/Weapons/Fireball/CDA_Fireball.CDA_Fireball"),
		TEXT("/Game/Weapons/Bomb/CDA_Bomb.CDA_Bomb"),
		TEXT("/Game/Weapons/Yondu/CDA_Yondu.CDA_Yondu"),
	};

	for (const TCHAR* const assetPath : DefaultWeaponDataPaths)
	{
		UCWeaponAsset* loadedAsset = TryLoadWeaponDataAssetFromContentPath(FString(assetPath));
		if (IsValid(loadedAsset))
		{
			DataAssets.AddUnique(loadedAsset);
			continue;
		}

		// .CDA_Fist suffix 없이 시도
		FString shortPath = FString(assetPath);
		const int32 dotIndex = shortPath.Find(TEXT("."), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		if (dotIndex != INDEX_NONE)
		{
			shortPath = shortPath.Left(dotIndex);
			loadedAsset = TryLoadWeaponDataAssetFromContentPath(shortPath);
			if (IsValid(loadedAsset))
			{
				DataAssets.AddUnique(loadedAsset);
				continue;
			}
		}

		CLog::Log(FString::Printf(
			TEXT("[Weapon] CDA 로드 실패 — %s (ActiveGameNameRedirects·Reparent=UCWeaponAsset 확인) Comp=%s"),
			assetPath,
			*GetName()));
	}

	if (DataAssets.Num() > 0)
	{
		CLog::LogDisplay(FString::Printf(
			TEXT("[Weapon] Content CDA_* 로드 %d개 — Comp=%s Owner=%s"),
			DataAssets.Num(),
			*GetName(),
			IsValid(ownerActor) ? *ownerActor->GetName() : TEXT("(null)")));
	}
	else
	{
		CreateSyntheticDataAssetsIfStillEmpty();
	}

	if (DataAssets.Num() < 1)
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] DataAssets 최종 0 — 합성 DA 생성도 실패 Comp=%s Owner=%s"),
			*GetName(),
			IsValid(ownerActor) ? *ownerActor->GetName() : TEXT("(null)")));
	}
}

void UCWeaponComponent::RebuildAssetMaps()
{
	RefreshOwnerCache();

	ACCommonCharacter* ownerChar = Owner.Get();
	if (false == IsValid(ownerChar))
		ownerChar = Cast<ACCommonCharacter>(GetOwner());

	if (false == IsValid(ownerChar))
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] RebuildAssetMaps: Owner 없음 — Comp=%s Actor=%s DataAssets=%d"),
			*GetName(),
			IsValid(GetOwner()) ? *GetOwner()->GetName() : TEXT("(null)"),
			DataAssets.Num()));
		return;
	}

	WeaponAssetMap.Empty();
	MagicAssetMap.Empty();

	for (int32 i = 0; i < DataAssets.Num(); i++)
	{
		if (IsValid(DataAssets[i]))
		{
			TObjectPtr<UCWeaponAsset> asset = NewObject<UCWeaponAsset>(this, UCWeaponAsset::StaticClass());

			asset->DeepCopy(*DataAssets[i], ownerChar);

			if (asset->GetMagicType() != CEMagicType::Unarmed)
				MagicAssetMap.Emplace(asset->GetMagicType(), asset);
			else
				WeaponAssetMap.Emplace(asset->GetType(), asset);
		}
	}

	CLog::LogDisplay(FString::Printf(
		TEXT("[Weapon] RebuildAssetMaps — Comp=%s DataAssets=%d WeaponMap=%d MagicMap=%d Owner=%s"),
		*GetName(),
		DataAssets.Num(),
		WeaponAssetMap.Num(),
		MagicAssetMap.Num(),
		*ownerChar->GetName()));
}

void UCWeaponComponent::EnsureDefaultMainWeaponClasses()
{
	if (MainWeaponClasses.Num() > 0)
	{
		return;
	}

	const ACPlayableCharacter* const playableOwner = Cast<ACPlayableCharacter>(GetOwner());
	if (false == IsValid(playableOwner))
	{
		return;
	}

	static const FMainWeaponSlotDefault SlotDefaults[] = {
		{ nullptr, nullptr },
		{ TEXT("/Game/Weapons/Fist/Combo_Fist.Combo_Fist_C"), ACWeaponComboFist::StaticClass() },
		{ TEXT("/Game/Weapons/Sword/RandomPattern_Sword.RandomPattern_Sword_C"), ACWeaponRandomPatternSword::StaticClass() },
		{ TEXT("/Game/Weapons/Hammer/Combo_Hammer.Combo_Hammer_C"), ACWeaponComboHammer::StaticClass() },
		{ TEXT("/Game/Weapons/Bow/Weapon_Bow.Weapon_Bow_C"), ACWeaponBow::StaticClass() },
		{ TEXT("/Game/Weapons/Dual/Combo_Dual.Combo_Dual_C"), ACWeaponComboDual::StaticClass() },
		{ nullptr, nullptr },
		{ nullptr, nullptr },
		{ nullptr, nullptr },
	};

	MainWeaponClasses.SetNum(WeaponEquipmentSlotCount);

	int32 filledCount = 0;
	for (int32 slotIndex = 0; slotIndex < WeaponEquipmentSlotCount; ++slotIndex)
	{
		const FMainWeaponSlotDefault& slotDefault = SlotDefaults[slotIndex];
		const TSubclassOf<ACWeapon> resolvedClass = ResolveMainWeaponClassForSlot(slotDefault);
		if (IsValid(resolvedClass))
		{
			MainWeaponClasses[slotIndex] = resolvedClass;
			++filledCount;
		}
	}

	CLog::LogDisplay(FString::Printf(
		TEXT("[Weapon] EnsureDefaultMainWeaponClasses — Comp=%s Owner=%s Filled=%d/%d"),
		*GetName(),
		*playableOwner->GetName(),
		filledCount,
		WeaponEquipmentSlotCount));
}

void UCWeaponComponent::EnsureSpawnedWeaponMatchesPhysicalType()
{
	if (bMagicEquipped)
	{
		return;
	}

	if (PhysicalType == CEWeaponType::Unarmed)
	{
		return;
	}

	const int32 slotIndex = static_cast<int32>(PhysicalType);
	if (false == MainWeapons.IsValidIndex(slotIndex))
	{
		return;
	}

	ACWeapon* const slotWeapon = MainWeapons[slotIndex].Get();
	if (false == IsValid(slotWeapon))
	{
		return;
	}

	if (MainWeapon.Get() == slotWeapon)
	{
		return;
	}

	MainWeapon = slotWeapon;
	DispatchLegacyEquipOrBeginEquipGate(MainWeapon, true);
}

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	EnsureWeaponPipelineReady();

	EnsureDefaultMainWeaponClasses();

	SpawnConfiguredWeapons();

	SyncBpWeaponLanes();

	EnsureCombatWeaponEquipped();

	EnsureSpawnedWeaponMatchesPhysicalType();

	LogWeaponPipelineStatus(TEXT("BeginPlay"));
}

void UCWeaponComponent::SpawnConfiguredWeapons()
{
	AActor* const OwnerActor = GetOwner();
	CheckNull(OwnerActor);

	// 클라에서는 동일 무기 세트 중복 생성을 막는다(BP 무기 블루프린트는 보통 레플리카가 아님).
	if (false == OwnerActor->HasAuthority())
	{
		return;
	}

	UWorld* const World = OwnerActor->GetWorld();
	CheckNull(World);

	const int32 MainConfiguredCount = MainWeaponClasses.Num();
	const int32 SubConfiguredCount = SubWeaponClasses.Num();
	const bool bHasAnySubConfigured = SubConfiguredCount > 0;

	int32 DesiredWeaponSlots = FMath::Max(WeaponEquipmentSlotCount, MainConfiguredCount);

	if (bHasAnySubConfigured)
	{
		DesiredWeaponSlots = FMath::Max(DesiredWeaponSlots, SubConfiguredCount);
	}

	MainWeapons.SetNumZeroed(DesiredWeaponSlots);
	SubWeapons.SetNumZeroed(DesiredWeaponSlots);

	// 마스터 BP: 메인 무기 클래스 foreach 가 기준 스트라이드. 서브만 길거나 메인 보다 긴 경우에는 위 크기 확장만 반영된다.
	for (int32 Idx = 0; Idx < MainConfiguredCount; ++Idx)
	{
		(void)TrySpawnOrReplaceEquippedWeapon(World, MainWeapons[Idx], MainWeaponClasses[Idx], OwnerActor);

		if (false == bHasAnySubConfigured)
		{
			continue;
		}

		const TSubclassOf<ACWeapon> SubWeaponClass =
			SubWeaponClasses.IsValidIndex(Idx) ? SubWeaponClasses[Idx] : TSubclassOf<ACWeapon>();

		(void)TrySpawnOrReplaceEquippedWeapon(World, SubWeapons[Idx], SubWeaponClass, OwnerActor);
	}

	// 레거시 BP 에서는 무기 각 슬롯 처리마다 Armor foreach 가 다시 실행되어 동일 블루프린트를 반복 소유했을 수 있다.
	// 여기서는 아머는 1패스만 초기화해 중복 Spawn 을 줄인다(Slot 은 레거시와 동일한 개수 확보).
	const int32 ArmorConfiguredCount = ArmorClasses.Num();

	if (ArmorConfiguredCount > 0)
	{
		Armors.SetNumZeroed(FMath::Max(Armors.Num(), ArmorConfiguredCount));

		for (int32 Idx = 0; Idx < ArmorConfiguredCount; ++Idx)
		{
			(void)TrySpawnOrReplaceEquippedActor(World, Armors[Idx], ArmorClasses[Idx], OwnerActor);
		}

		if (Armor == nullptr && Armors.IsValidIndex(0))
		{
			Armor = Armors[0];
		}
	}

	if (MainWeapon == nullptr && MainWeapons.IsValidIndex(0))
	{
		MainWeapon = MainWeapons[0];
	}

	if (SubWeapon == nullptr && SubWeapons.IsValidIndex(0))
	{
		SubWeapon = SubWeapons[0];
	}
}

bool UCWeaponComponent::IsUnarmed() const
{
	return MainType == CEWeaponType::Unarmed;
}

bool UCWeaponComponent::IsBow() const
{
	return MainType == CEWeaponType::Bow;
}

void UCWeaponComponent::GetMainType(CEWeaponType& OutMainType) const
{
	OutMainType = MainType;
}

void UCWeaponComponent::ChangeBlueprintWeaponLanes(CEWeaponType InNewMainType, CEWeaponType InNewSubType)
{
	const CEWeaponType PrevMainType = MainType;
	const CEWeaponType PrevSubType = SubType;

	MainType = InNewMainType;
	SubType = InNewSubType;

	// 레거시 BP 무기 레인(MainType/SubType) 과 PhysicalType 을 동기화하지 않으면 InputAction_Act → GetWeaponAsset 이
	// PhysicalType==Unarmed 만 보고 무기가 착장된 상태에서도 GetAct()==null 로 무반응된다(블프 SetMode 후 LMB 불능).
	if (false == bMagicEquipped)
	{
		if (MainType == CEWeaponType::Unarmed)
		{
			PhysicalType = CEWeaponType::Unarmed;
			LastCommittedPhysical = CEWeaponType::Unarmed;
		}
		else
		{
			PhysicalType = MainType;
			LastCommittedPhysical = MainType;
		}
	}

	if (OnWeaponTypeChanged.IsBound())
		OnWeaponTypeChanged.Broadcast(PrevMainType, MainType, PrevSubType, SubType);
}

void UCWeaponComponent::SetBlueprintUnarmed_WithSpawnedWeapons()
{
	DispatchLegacyUnequipOrEndUnequipGate(MainWeapon, true);
	DispatchLegacyUnequipOrEndUnequipGate(SubWeapon, false);

	ChangeBlueprintWeaponLanes(CEWeaponType::Unarmed, CEWeaponType::Unarmed);
}

void UCWeaponComponent::SetBlueprintSpawnedWeaponMode(CEWeaponType InMainType, CEWeaponType InSubType)
{
	// BP: 요청 무기 종류와 현재 MainType 이 같으면 전부 Unequip + Unarmed 레인 전환만 수행한다(토글 해방 규약).
	if (InMainType == MainType)
	{
		SetBlueprintUnarmed_WithSpawnedWeapons();
		return;
	}

	const bool bMainLaneWasUnarmed = (MainType == CEWeaponType::Unarmed);

	// 현재 무장 중이었다면 순서대로 주·보조 BP Weapon 을 레거시 Unequip(또는 bool 게이트 포함 End_Unequip) 순으로 처리한다.
	if (false == bMainLaneWasUnarmed)
	{
		DispatchLegacyUnequipOrEndUnequipGate(MainWeapon, true);
		DispatchLegacyUnequipOrEndUnequipGate(SubWeapon, false);
	}

	const int32 SlotIndex = static_cast<int32>(InMainType);

	if (MainWeapons.IsValidIndex(SlotIndex))
		MainWeapon = MainWeapons[SlotIndex];
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CWeaponComponent][SetBlueprintSpawnedWeaponMode] MainWeapons 인덱스 없음 Idx=%d — CEWeaponType 값과 배열 슬롯을 BP 와 동일 순서로 맞춘다."),
			SlotIndex);
		MainWeapon = nullptr;
	}

	if (IsValid(MainWeapon))
		DispatchLegacyEquipOrBeginEquipGate(MainWeapon, true);

	if (SubWeapons.IsValidIndex(SlotIndex))
		SubWeapon = SubWeapons[SlotIndex];
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CWeaponComponent][SetBlueprintSpawnedWeaponMode] SubWeapons 인덱스 없음 Idx=%d."), SlotIndex);
		SubWeapon = nullptr;
	}

	// BP SetMode 그래프는 SubWeapon 슬롯만 갈아 끼우고 Begin_Equip/Equip 호출은 MainWeapon 에만 준다.
	ChangeBlueprintWeaponLanes(InMainType, InSubType);
}

void UCWeaponComponent::Blueprint_SetSword_WithSpawnedWeapons()
{
	// BP 에서 서브 무기 타입 기본값 NewEnumerator9: 원 에셋 EWeaponType 열 순서가 다르면 여기를 조정한다(현 재단에서는 Shield 레인을 사용).
	SetBlueprintSpawnedWeaponMode(CEWeaponType::Sword, CEWeaponType::Shield);
}

void UCWeaponComponent::Blueprint_SetFist_WithSpawnedWeapons()
{
	SetBlueprintSpawnedWeaponMode(CEWeaponType::Fist, CEWeaponType::Unarmed);
}

void UCWeaponComponent::Blueprint_SetHammer_WithSpawnedWeapons()
{
	SetBlueprintSpawnedWeaponMode(CEWeaponType::Hammer, CEWeaponType::Unarmed);
}

void UCWeaponComponent::Blueprint_SetBow_WithSpawnedWeapons()
{
	SetBlueprintSpawnedWeaponMode(CEWeaponType::Bow, CEWeaponType::Unarmed);
}

void UCWeaponComponent::Blueprint_SetDual_WithSpawnedWeapons()
{
	SetBlueprintSpawnedWeaponMode(CEWeaponType::Dual, CEWeaponType::Unarmed);
}

void UCWeaponComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const TWeakObjectPtr<UCAct> act = GetAct();
	if (act.IsValid())
		act->Tick(DeltaTime);

	int32 index = 0;
	TWeakObjectPtr<UCSkill> skill = GetSkill(index);

	while (skill.IsValid())
	{
		skill->Tick(DeltaTime);

		index++;
		skill = GetSkill(index);
	}
}

void UCWeaponComponent::InputAction_Act()
{
	EnsureWeaponPipelineReady();
	SyncBpWeaponLanes();
	EnsureCombatWeaponEquipped();
	EnsureSpawnedWeaponMatchesPhysicalType();

	const TWeakObjectPtr<UCAct> act = GetAct();
	if (false == act.IsValid())
	{
		const CEWeaponType lookupType = ResolveCombatWeaponLookupType();
		const AActor* const ownerActor = GetOwner();
		const TWeakObjectPtr<UCWeaponAsset> asset = GetWeaponAsset();

		if (asset.IsValid() && (false == IsValid(asset->GetAct())) && Owner.IsValid())
		{
			asset->BeginPlay(Owner);
		}

		const TWeakObjectPtr<UCAct> actRetry = GetAct();
		if (actRetry.IsValid())
		{
			actRetry->Act();
			return;
		}

		LogWeaponPipelineStatus(TEXT("ActionFail"));
		CLog::Log(FString::Printf(
			TEXT("[입력][Action] UCAct 없음 — Physical=%s Lookup=%s Equip=%s Map=%d MagicMap=%d DataAssets=%d ActDatas=%d State=%s Owner=%s"),
			*YJJHelpers::ConvertEnumToString(PhysicalType),
			*YJJHelpers::ConvertEnumToString(lookupType),
			(IsValid(GetEquipment()) ? TEXT("O") : TEXT("X")),
			WeaponAssetMap.Num(),
			MagicAssetMap.Num(),
			DataAssets.Num(),
			(asset.IsValid() && IsValid(asset.Get()) ? asset->GetActDatasNum() : -1),
			(StateComp.IsValid() ? *YJJHelpers::ConvertEnumToString(StateComp->GetCurMode()) : TEXT("(null)")),
			IsValid(ownerActor) ? *ownerActor->GetName() : TEXT("(null)")));
		return;
	}

	act->Act();
}

void UCWeaponComponent::InputAction_SubAct_Pressed()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(0);
	CheckNull(skill);
	skill->Pressed();
}

void UCWeaponComponent::InputAction_SubAct_Released()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(0);
	CheckNull(skill);
	skill->Released();
}

void UCWeaponComponent::InputAction_Skill_1_Pressed()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(1);
	CheckNull(skill);
	skill->Pressed();
}

void UCWeaponComponent::InputAction_Skill_1_Released()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(1);
	CheckNull(skill);
	skill->Released();
}

void UCWeaponComponent::InputAction_Skill_2_Pressed()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(2);
	CheckNull(skill);
	skill->Pressed();
}

void UCWeaponComponent::InputAction_Skill_2_Released()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(2);
	CheckNull(skill);
	skill->Released();
}

void UCWeaponComponent::InputAction_Skill_3_Pressed()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(3);
	CheckNull(skill);
	skill->Pressed();
}

void UCWeaponComponent::InputAction_Skill_3_Released()
{
	const TWeakObjectPtr<UCSkill> skill = GetSkill(3);
	CheckNull(skill);
	skill->Released();
}

void UCWeaponComponent::SetModeFromZeroIndex()
{
	CheckFalse(WeaponAssetMap.Num() > 0);
	for (const TPair<CEWeaponType, TObjectPtr<UCWeaponAsset>>& Pair : WeaponAssetMap)
	{
		SetMode(Pair.Key);
		return;
	}
}

void UCWeaponComponent::ChangePhysicalType(CEWeaponType InNewPhysical)
{
	const CEWeaponType PrevBroadcastMain = PhysicalType;
	const CEWeaponType PrevBroadcastSub = SubType;

	PrevType = PhysicalType;
	PhysicalType = InNewPhysical;

	if (PhysicalType != CEWeaponType::Unarmed)
		LastCommittedPhysical = PhysicalType;

	MainType = PhysicalType;

	// 애니 Sub 레이어: 직전에 착용하던 물리 주무기(구 PrevType 디스패치 시그널과 동일 근거).
	SubType = PrevBroadcastMain;

	if (OnWeaponTypeChanged.IsBound())
		OnWeaponTypeChanged.Broadcast(PrevBroadcastMain, MainType, PrevBroadcastSub, SubType);
}

void UCWeaponComponent::SetMode(CEWeaponType InType)
{
	if (InType == CEWeaponType::Unarmed)
	{
		SetUnarmedMode();
		return;
	}

	if (false == bMagicEquipped && PhysicalType == InType)
	{
		SetUnarmedMode();
		return;
	}

	if (bMagicEquipped)
	{
		if (IsValid(GetEquipment()))
			GetEquipment()->Unequip();
		const CEMagicType prevMagic = MagicEquipType;
		bMagicEquipped = false;
		MagicEquipType = CEMagicType::Unarmed;
		if (prevMagic != CEMagicType::Unarmed)
			DispatchEquippedMagicDelegates(CEMagicType::Unarmed, prevMagic);
	}

	if (false == IsUnarmedPhysical())
	{
		CheckNull(GetEquipment());
		GetEquipment()->Unequip();
	}

	const TObjectPtr<UCWeaponAsset>* assetForCheckingNull = WeaponAssetMap.Find(InType);
	CheckNull(assetForCheckingNull);

	const TWeakObjectPtr<UCWeaponAsset> asset = *assetForCheckingNull;
	CheckNull(asset);

	const TWeakObjectPtr<UCEquipment> equipment = asset->GetEquipment();
	CheckNull(equipment);

	equipment->Equip();

	ChangePhysicalType(InType);
}

void UCWeaponComponent::SetMagicMode(CEMagicType InMagicType)
{
	CheckNull(Owner);

	if (InMagicType == CEMagicType::Unarmed)
	{
		if (IsValid(GetEquipment()))
			GetEquipment()->Unequip();

		const CEMagicType prevMagic = MagicEquipType;
		bMagicEquipped = false;
		MagicEquipType = CEMagicType::Unarmed;
		if (prevMagic != CEMagicType::Unarmed)
			DispatchEquippedMagicDelegates(CEMagicType::Unarmed, prevMagic);

		SetUnarmedMode();
		return;
	}

	if (bMagicEquipped && MagicEquipType == InMagicType)
	{
		if (IsValid(GetEquipment()))
			GetEquipment()->Unequip();

		const CEMagicType prevMagic = MagicEquipType;
		bMagicEquipped = false;
		MagicEquipType = CEMagicType::Unarmed;
		if (prevMagic != CEMagicType::Unarmed)
			DispatchEquippedMagicDelegates(CEMagicType::Unarmed, prevMagic);

		SetUnarmedMode();
		return;
	}

	if (IsValid(GetEquipment()))
		GetEquipment()->Unequip();

	const CEMagicType prevMagic = MagicEquipType;

	const TObjectPtr<UCWeaponAsset>* assetPtr = MagicAssetMap.Find(InMagicType);
	CheckNull(assetPtr);

	const TWeakObjectPtr<UCWeaponAsset> asset = *assetPtr;
	CheckNull(asset);

	const TWeakObjectPtr<UCEquipment> equipment = asset->GetEquipment();
	CheckNull(equipment);

	equipment->Equip();

	bMagicEquipped = true;
	MagicEquipType = InMagicType;
	PhysicalType = CEWeaponType::Unarmed;

	DispatchEquippedMagicDelegates(MagicEquipType, prevMagic);
}

void UCWeaponComponent::CancelAct()
{
	CheckNull(GetAct());
	GetAct()->End_Act();
}

void UCWeaponComponent::End_DoAction(CEAttackType InAttackType)
{
	// 레거시 BP 인자 이름만 맞춘다 — UCAct 분기에서는 CEAttackType 을 참조하지 않는다.
	(void)InAttackType;

	const TObjectPtr<UCAct> act = GetAct();
	if (false == IsValid(act))
	{
		// 데이터 에셋 Act 미부착 구간 등 — 애니 공용 종료 노티 경로라 로그는 남기지 않는다.
		return;
	}
	act->End_Act();
}

void UCWeaponComponent::Begin_DoAction(CEAttackType InAttackType)
{
	const TObjectPtr<AActor> ownerActor = GetOwner();
	if (false == IsValid(ownerActor))
	{
		return;
	}

	const TObjectPtr<UCMagicComponent> magicComp = ownerActor->FindComponentByClass<UCMagicComponent>();
	if (IsValid(magicComp))
	{
		if (magicComp->IsUnarmed())
		{
			LegacyBp_DispatchMain_BeginDoAction(InAttackType);
		}
		else
		{
			magicComp->Begin_DoAction(InAttackType);
		}
		return;
	}

	LegacyBp_DispatchMain_BeginDoAction(InAttackType);
}

void UCWeaponComponent::Begin_DoAirCombo()
{
	LegacyBp_DispatchMain_BeginDoAction(CEAttackType::Air);
}

void UCWeaponComponent::End_DoAirCombo()
{
	LegacyBp_DispatchMain_EndDoAction(CEAttackType::Air);
}

void UCWeaponComponent::Begin_DoFlyingAttack()
{
	const TObjectPtr<ACWeaponCombo> comboWeapon = Cast<ACWeaponCombo>(MainWeapon);
	if (IsValid(comboWeapon))
	{
		comboWeapon->Combo_BeginFlyingSegmentFromNotify();
		return;
	}

	LegacyBp_DispatchMain_BeginDoAction(CEAttackType::Flying);
}

void UCWeaponComponent::End_DoFlyingAttack()
{
	LegacyBp_DispatchMain_EndDoAction(CEAttackType::Flying);
}

void UCWeaponComponent::End_DoDownAttack()
{
	LegacyBp_DispatchMain_EndDoAction(CEAttackType::Down);
}

void UCWeaponComponent::End_DashAttack()
{
	LegacyBp_DispatchMain_EndDoAction(CEAttackType::DashAttack);
}

void UCWeaponComponent::End_FallDown()
{
	LegacyBp_DispatchMain_EndDoAction(CEAttackType::FallDown);
}

bool UCWeaponComponent::TryNotifyConsumeStamina(const double InStamina)
{
	ACWeapon* weapon = Cast<ACWeapon>(MainWeapon);
	if (false == IsValid(weapon))
	{
		// WeaponComponent 유효·MainWeapon 없음 — 레거시 IsValid 분기와 동일하게 소모 없이 통과.
		return true;
	}

	const bool bEnoughStamina = weapon->ConsumeStamina(InStamina);
	if (bEnoughStamina)
	{
		return true;
	}

	End_DoAction(CEAttackType::Common);

	ACharacter* character = Cast<ACharacter>(GetOwner());
	if (IsValid(character))
	{
		character->StopAnimMontage(nullptr);
	}

	return false;
}

void UCWeaponComponent::ApplyLegacyMainWeaponBoxCollisions()
{
	ACWeaponCombo* const ComboWeapon = Cast<ACWeaponCombo>(MainWeapon);
	if (false == IsValid(ComboWeapon))
	{
		return;
	}

	if (ICombatActionHost* const Host = Cast<ICombatActionHost>(ComboWeapon))
	{
		Host->Host_OnBoxCollisions();
	}
}

void UCWeaponComponent::ApplyLegacyEndBowStringAttach()
{
	ACWeaponBow* bowWeapon = Cast<ACWeaponBow>(MainWeapon);
	if (false == IsValid(bowWeapon))
	{
		return;
	}

	bowWeapon->AttachBowString = true;
}

bool UCWeaponComponent::TryDispatchLegacyMainWeaponCollisionToggle(const bool bCollisionOn)
{
	if (TryDispatchWeaponCollisionHost(MainWeapon, bCollisionOn))
	{
		return true;
	}

	if (false == IsValid(MainWeapon))
	{
		return false;
	}

	const FName functionNameLocal = bCollisionOn ? FName(TEXT("OnCollisions")) : FName(TEXT("OffCollisions"));
	UFunction* functionPtrLocal = MainWeapon->FindFunction(functionNameLocal);
	if (nullptr == functionPtrLocal)
	{
		return false;
	}

	MainWeapon->ProcessEvent(functionPtrLocal, nullptr);
	return true;
}

void UCWeaponComponent::ApplyLegacyMainWeaponCollisionBound(const bool bCollisionOn)
{
	if (TryDispatchLegacyMainWeaponCollisionToggle(bCollisionOn))
	{
		return;
	}

	const TObjectPtr<ACAttachment> attachment = GetAttachment();
	if (false == IsValid(attachment))
	{
		return;
	}

	if (bCollisionOn)
	{
		attachment->OnCollisions();
	}
	else
	{
		attachment->OffCollisions();
	}
}

void UCWeaponComponent::ApplyLegacyMainWeaponComboWindow(const bool bEnableCombo)
{
	if (IsValid(MainWeapon))
	{
		if (Cast<ACWeaponCombo>(MainWeapon) || Cast<ACWeaponRandomPattern>(MainWeapon))
		{
			if (ICombatActionHost* const Host = Cast<ICombatActionHost>(MainWeapon))
			{
				if (bEnableCombo)
				{
					Host->Host_EnableCombo();
				}
				else
				{
					Host->Host_DisableCombo();
				}

				return;
			}
		}
	}

	UCAct* const Act = GetAct().Get();
	if (IsValid(Act))
	{
		if (ICombatActionHost* const ActHost = Cast<ICombatActionHost>(Act))
		{
			if (bEnableCombo)
			{
				ActHost->Host_EnableCombo();
			}
			else
			{
				ActHost->Host_DisableCombo();
			}
		}
	}
}

void UCWeaponComponent::LegacyBp_DispatchMain_DoAction(CEAttackType InAttackType, int32 InSkillIndex)
{
	if (IsValid(MainWeapon))
	{
		if (ICombatActionHost* const Host = Cast<ICombatActionHost>(MainWeapon))
		{
			Host->Host_DoAction(InAttackType, InSkillIndex);
			return;
		}
	}

	if (false == IsValid(MainWeapon))
	{
		return;
	}

	static const FName DoActionName(TEXT("DoAction"));
	if (nullptr == MainWeapon->FindFunction(DoActionName))
	{
		UE_LOG(LogTemp, Verbose, TEXT("[CWeaponComponent][LegacyBp_DispatchMain_DoAction] DoAction 미정의 — %s"), *MainWeapon->GetName());

		return;
	}

	(void)TryDispatch_ProcessEvent_WithAttackSkillIndexParms(MainWeapon, DoActionName, InAttackType, InSkillIndex);
}

void UCWeaponComponent::LegacyBp_DispatchMain_BeginDoAction(CEAttackType InAttackType)
{
	if (IsValid(MainWeapon))
	{
		if (ICombatActionHost* const Host = Cast<ICombatActionHost>(MainWeapon))
		{
			Host->Host_BeginDoAction(InAttackType);
			return;
		}
	}

	if (false == IsValid(MainWeapon))
	{
		return;
	}

	static const FName Begin(TEXT("Begin_DoAction"));
	if (false == TryDispatch_ProcessEvent_AttackEnumOnlyParms_WithValue(MainWeapon, Begin, InAttackType))
	{
		UE_LOG(LogTemp, Verbose, TEXT("[CWeaponComponent][LegacyBp_DispatchMain_BeginDoAction] Begin_DoAction 호출 불가 — %s"), *MainWeapon->GetName());
	}
}

void UCWeaponComponent::LegacyBp_DispatchMain_EndDoAction(CEAttackType InAttackType)
{
	if (IsValid(MainWeapon))
	{
		if (ICombatActionHost* const Host = Cast<ICombatActionHost>(MainWeapon))
		{
			Host->Host_EndDoAction(InAttackType);
			return;
		}
	}

	if (false == IsValid(MainWeapon))
	{
		return;
	}

	static const FName End(TEXT("End_DoAction"));
	if (false == TryDispatch_ProcessEvent_AttackEnumOnlyParms_WithValue(MainWeapon, End, InAttackType))
	{
		UE_LOG(LogTemp, Verbose, TEXT("[CWeaponComponent][LegacyBp_DispatchMain_EndDoAction] End_DoAction 호출 불가 — %s"), *MainWeapon->GetName());
	}
}

void UCWeaponComponent::LegacyBp_DispatchMain_Pressed()
{
	TryLegacyWeaponProcessEventIfBound(MainWeapon, FName(TEXT("Pressed")));
}

void UCWeaponComponent::LegacyBp_DispatchMain_Released()
{
	TryLegacyWeaponProcessEventIfBound(MainWeapon, FName(TEXT("Released")));
}

void UCWeaponComponent::LegacyBp_DispatchMain_Dash()
{
	TryLegacyWeaponProcessEventIfBound(MainWeapon, FName(TEXT("Dash")));
}

void UCWeaponComponent::LegacyBp_DispatchMain_EndGroundDash()
{
	TryLegacyWeaponProcessEventIfBound(MainWeapon, FName(TEXT("End_GroundDash")));
}

void UCWeaponComponent::LegacyBp_DispatchMain_AirDash()
{
	TryLegacyWeaponProcessEventIfBound(MainWeapon, FName(TEXT("AirDash")));
}

void UCWeaponComponent::LegacyBp_DispatchMain_EndAirDash()
{
	TryLegacyWeaponProcessEventIfBound(MainWeapon, FName(TEXT("End_AirDash")));
}

void UCWeaponComponent::LegacyBp_DispatchMain_Skill(const int32 InSkillIndex)
{
	if (IsValid(MainWeapon))
	{
		if (ACWeaponComboVisual* const VisualWeapon = Cast<ACWeaponComboVisual>(MainWeapon))
		{
			if (ICombatActionHost* const Host = Cast<ICombatActionHost>(VisualWeapon))
			{
				Host->Host_Skill(InSkillIndex);
				return;
			}
		}
	}

	if (false == IsValid(MainWeapon))
	{
		return;
	}

	static const FName SkillName(TEXT("Skill"));
	if (TryDispatch_ProcessEvent_FirstInt32Parm(MainWeapon, SkillName, InSkillIndex))
	{
		return;
	}

	// Skill 커스텀 이벤트가 없으면 DoAction(Skill, Index) 규약으로 폴백한다.
	(void)TryDispatch_ProcessEvent_WithAttackSkillIndexParms(MainWeapon, FName(TEXT("DoAction")), CEAttackType::Skill, InSkillIndex);
}

void UCWeaponComponent::LegacyBp_DispatchMain_EndSkill()
{
	if (IsValid(MainWeapon))
	{
		if (ACWeaponComboVisual* const VisualWeapon = Cast<ACWeaponComboVisual>(MainWeapon))
		{
			if (ICombatActionHost* const Host = Cast<ICombatActionHost>(VisualWeapon))
			{
				Host->Host_EndSkill();
				return;
			}
		}
	}

	TryLegacyWeaponProcessEventIfBound(MainWeapon, FName(TEXT("End_Skill")));
}

void UCWeaponComponent::LegacyBp_DispatchSub_HoldSubWeapon()
{
	TryLegacyWeaponProcessEventIfBound(SubWeapon, FName(TEXT("Hold_SubWeapon")));
}

void UCWeaponComponent::LegacyBp_DispatchSub_ReleasedSubWeapon()
{
	TryLegacyWeaponProcessEventIfBound(SubWeapon, FName(TEXT("Released_SubWeapon")));
}

void UCWeaponComponent::LegacyBp_DispatchSub_DoSubWeaponAction(CEAttackType InAttackType, int32 InSkillIndex)
{
	if (false == IsValid(SubWeapon))
		return;

	static const FName Fn(TEXT("DoSubWeaponAction"));

	if (TryDispatch_ProcessEvent_WithAttackSkillIndexParms(SubWeapon, Fn, InAttackType, InSkillIndex))
		return;

	(void)TryDispatch_ProcessEvent_WithAttackSkillIndexParms(SubWeapon, FName(TEXT("DoAction")), InAttackType, InSkillIndex);
}

void UCWeaponComponent::LegacyBp_DispatchSub_BeginDoSubWeaponAction(CEAttackType InAttackType)
{
	if (false == IsValid(SubWeapon))
		return;

	static const FName Fn(TEXT("Begin_DoSubWeaponAction"));
	if (false == TryDispatch_ProcessEvent_AttackEnumOnlyParms_WithValue(SubWeapon, Fn, InAttackType))
	{
		(void)TryDispatch_ProcessEvent_AttackEnumOnlyParms_WithValue(SubWeapon, FName(TEXT("Begin_DoAction")), InAttackType);
	}
}

void UCWeaponComponent::LegacyBp_DispatchSub_EndDoSubWeaponAction(CEAttackType InAttackType)
{
	if (false == IsValid(SubWeapon))
		return;

	static const FName Fn(TEXT("End_DoSubWeaponAction"));
	if (false == TryDispatch_ProcessEvent_AttackEnumOnlyParms_WithValue(SubWeapon, Fn, InAttackType))
	{
		(void)TryDispatch_ProcessEvent_AttackEnumOnlyParms_WithValue(SubWeapon, FName(TEXT("End_DoAction")), InAttackType);
	}
}

void UCWeaponComponent::Begin_Equip()
{
	DispatchLegacyEquipOrBeginEquipGate(MainWeapon, true);
	DispatchLegacyEquipOrBeginEquipGate(SubWeapon, false);
}

void UCWeaponComponent::End_Equip()
{
	DispatchLegacyEndEquipGate(MainWeapon, true);
	DispatchLegacyEndEquipGate(SubWeapon, false);
}

void UCWeaponComponent::Begin_Unequip()
{
	DispatchLegacyBeginUnequipGate(MainWeapon, true);
	DispatchLegacyBeginUnequipGate(SubWeapon, false);
}

void UCWeaponComponent::End_Unequip()
{
	DispatchLegacyUnequipOrEndUnequipGate(MainWeapon, true);
	DispatchLegacyUnequipOrEndUnequipGate(SubWeapon, false);
}

bool UCWeaponComponent::IsIdleStateMode()
{
	StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());
	CheckNullResult(StateComp, false);
	return StateComp->IsIdleMode();
}

void UCWeaponComponent::SetUnarmedMode()
{
	if (bMagicEquipped)
	{
		if (IsValid(GetEquipment()))
			GetEquipment()->Unequip();
		const CEMagicType prevMagic = MagicEquipType;
		bMagicEquipped = false;
		MagicEquipType = CEMagicType::Unarmed;
		if (prevMagic != CEMagicType::Unarmed)
			DispatchEquippedMagicDelegates(CEMagicType::Unarmed, prevMagic);
	}

	if (IsValid(GetEquipment()))
		GetEquipment()->Unequip();

	LastCommittedPhysical = CEWeaponType::Unarmed;
	ChangePhysicalType(CEWeaponType::Unarmed);
}

void UCWeaponComponent::SetFistMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Fist);
}

void UCWeaponComponent::SetSwordMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Sword);
}

void UCWeaponComponent::SetHammerMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Hammer);
}

void UCWeaponComponent::SetBowMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Bow);
}

void UCWeaponComponent::SetDualMode()
{
	CheckFalse(IsIdleStateMode());
	SetMode(CEWeaponType::Dual);
}

void UCWeaponComponent::SetWarpMode()
{
	CheckFalse(IsIdleStateMode());
	SetMagicMode(CEMagicType::Warp);
}

void UCWeaponComponent::SetAroundMode()
{
	CheckFalse(IsIdleStateMode());
	SetMagicMode(CEMagicType::Around);
}

void UCWeaponComponent::SetFireballMode()
{
	CheckFalse(IsIdleStateMode());
	SetMagicMode(CEMagicType::FireBall);
}

void UCWeaponComponent::SetBombMode()
{
	CheckFalse(IsIdleStateMode());
	SetMagicMode(CEMagicType::Bomb);
}

void UCWeaponComponent::SetYonduMode()
{
	CheckFalse(IsIdleStateMode());
	SetMagicMode(CEMagicType::Yondu);
}

TWeakObjectPtr<UCWeaponAsset> UCWeaponComponent::GetWeaponAsset()
{
	if (bMagicEquipped)
	{
		const TObjectPtr<UCWeaponAsset>* foundMagic = MagicAssetMap.Find(MagicEquipType);
		CheckNullResult(foundMagic, nullptr);
		const TWeakObjectPtr<UCWeaponAsset> magicAsset = *foundMagic;
		return magicAsset;
	}

	const CEWeaponType lookupType = ResolveCombatWeaponLookupType();
	if (lookupType == CEWeaponType::Unarmed)
		return nullptr;

	const TObjectPtr<UCWeaponAsset>* foundWeapon = WeaponAssetMap.Find(lookupType);
	CheckNullResult(foundWeapon, nullptr);

	const TWeakObjectPtr<UCWeaponAsset> wAsset = *foundWeapon;
	return wAsset;
}

TObjectPtr<ACAttachment> UCWeaponComponent::GetAttachment()
{
	const TWeakObjectPtr<UCWeaponAsset> asset = GetWeaponAsset();
	CheckNullResult(asset, nullptr);

	return asset->GetAttachment();
}

TObjectPtr<UCEquipment> UCWeaponComponent::GetEquipment()
{
	const TWeakObjectPtr<UCWeaponAsset> asset = GetWeaponAsset();
	CheckNullResult(asset, nullptr);

	return asset->GetEquipment();
}

TObjectPtr<UCAct> UCWeaponComponent::GetAct()
{
	const TWeakObjectPtr<UCWeaponAsset> asset = GetWeaponAsset();
	CheckNullResult(asset, nullptr);

	return asset->GetAct();
}

TObjectPtr<UCSkill> UCWeaponComponent::GetSkill(const int32 SkillIndex)
{
	const TWeakObjectPtr<UCWeaponAsset> asset = GetWeaponAsset();
	CheckNullResult(asset, nullptr);

	return asset->GetSkill(SkillIndex);
}
