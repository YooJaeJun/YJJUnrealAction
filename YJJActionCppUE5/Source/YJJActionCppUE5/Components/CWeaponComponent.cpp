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

namespace
{
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
	static const FName UnequipName(TEXT("Unequip"));

	TryLegacyWeaponProcessEventIfBound(MainWeapon, UnequipName);
	TryLegacyWeaponProcessEventIfBound(SubWeapon, UnequipName);

	ChangeBlueprintWeaponLanes(CEWeaponType::Unarmed, CEWeaponType::Unarmed);
}

void UCWeaponComponent::SetBlueprintSpawnedWeaponMode(CEWeaponType InMainType, CEWeaponType InSubType)
{
	static const FName EquipName(TEXT("Equip"));
	static const FName UnequipName(TEXT("Unequip"));

	// BP: 요청 무기 종류와 현재 MainType 이 같으면 전부 Unequip + Unarmed 레인 전환만 수행한다(토글 해방 규약).
	if (InMainType == MainType)
	{
		SetBlueprintUnarmed_WithSpawnedWeapons();
		return;
	}

	const bool bMainLaneWasUnarmed = (MainType == CEWeaponType::Unarmed);

	// 현재 무장 중이었다면 순서대로 주·보조 BP Weapon 을 Unequip.
	if (false == bMainLaneWasUnarmed)
	{
		TryLegacyWeaponProcessEventIfBound(MainWeapon, UnequipName);
		TryLegacyWeaponProcessEventIfBound(SubWeapon, UnequipName);
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
		TryLegacyWeaponProcessEventIfBound(MainWeapon, EquipName);

	if (SubWeapons.IsValidIndex(SlotIndex))
		SubWeapon = SubWeapons[SlotIndex];
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[CWeaponComponent][SetBlueprintSpawnedWeaponMode] SubWeapons 인덱스 없음 Idx=%d."), SlotIndex);
		SubWeapon = nullptr;
	}

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
