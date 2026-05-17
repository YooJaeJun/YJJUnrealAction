#include "Components/CWeaponComponent.h"
#include "Global.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CMagicComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CEquipment.h"
#include "Weapons/CAct.h"
#include "Weapons/CSkill.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/UnrealType.h"
#include "UObject/EnumProperty.h"

namespace
{
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

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	CheckNull(Owner);

	for (int32 i = 0; i < DataAssets.Num(); i++)
	{
		if (IsValid(DataAssets[i]))
		{
			// MagicType 과 Type 을 함께 채운 DA 는 Magic 쪽 우선이라 물리 Weapon 맵에는 넣히지 않는다. 한 축만 지정한다.
			TObjectPtr<UCWeaponAsset> asset = NewObject<UCWeaponAsset>(this, UCWeaponAsset::StaticClass());

			asset->DeepCopy(*DataAssets[i], Owner);

			if (asset->GetMagicType() != CEMagicType::Unarmed)
				MagicAssetMap.Emplace(asset->GetMagicType(), asset);
			else
				WeaponAssetMap.Emplace(asset->GetType(), asset);
		}
	}

	SpawnEquippedActorsFromConfiguredClasses();
}

void UCWeaponComponent::SpawnEquippedActorsFromConfiguredClasses()
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
		(void)TrySpawnOrReplaceEquippedActor(World, MainWeapons[Idx], MainWeaponClasses[Idx], OwnerActor);

		if (false == bHasAnySubConfigured)
		{
			continue;
		}

		const TSubclassOf<AActor> SubWeaponClass =
			SubWeaponClasses.IsValidIndex(Idx) ? SubWeaponClasses[Idx] : TSubclassOf<AActor>();

		(void)TrySpawnOrReplaceEquippedActor(World, SubWeapons[Idx], SubWeaponClass, OwnerActor);
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
	const TWeakObjectPtr<UCAct> act = GetAct();
	CheckNull(act);
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

bool UCWeaponComponent::TryDispatchLegacyMainWeaponCollisionToggle(const bool bCollisionOn)
{
	if (false == IsValid(MainWeapon))
		return false;

	const FName functionNameLocal = bCollisionOn ? FName(TEXT("OnCollisions")) : FName(TEXT("OffCollisions"));
	UFunction* functionPtrLocal = MainWeapon->FindFunction(functionNameLocal);
	if (nullptr == functionPtrLocal)
		return false;

	MainWeapon->ProcessEvent(functionPtrLocal, nullptr);
	return true;
}

void UCWeaponComponent::LegacyBp_DispatchMain_DoAction(CEAttackType InAttackType, int32 InSkillIndex)
{
	if (false == IsValid(MainWeapon))
		return;

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
	if (false == IsValid(MainWeapon))
		return;

	static const FName Begin(TEXT("Begin_DoAction"));
	if (false == TryDispatch_ProcessEvent_AttackEnumOnlyParms_WithValue(MainWeapon, Begin, InAttackType))
	{
		UE_LOG(LogTemp, Verbose, TEXT("[CWeaponComponent][LegacyBp_DispatchMain_BeginDoAction] Begin_DoAction 호출 불가 — %s"), *MainWeapon->GetName());
	}
}

void UCWeaponComponent::LegacyBp_DispatchMain_EndDoAction(CEAttackType InAttackType)
{
	if (false == IsValid(MainWeapon))
		return;

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
	if (false == IsValid(MainWeapon))
		return;

	static const FName SkillName(TEXT("Skill"));
	if (TryDispatch_ProcessEvent_FirstInt32Parm(MainWeapon, SkillName, InSkillIndex))
		return;

	// Skill 커스텀 이벤트가 없으면 DoAction(Skill, Index) 규약으로 폴백한다.
	(void)TryDispatch_ProcessEvent_WithAttackSkillIndexParms(MainWeapon, FName(TEXT("DoAction")), CEAttackType::Skill, InSkillIndex);
}

void UCWeaponComponent::LegacyBp_DispatchMain_EndSkill()
{
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

	if (PhysicalType == CEWeaponType::Unarmed)
		return nullptr;

	const TObjectPtr<UCWeaponAsset>* foundWeapon = WeaponAssetMap.Find(PhysicalType);
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
