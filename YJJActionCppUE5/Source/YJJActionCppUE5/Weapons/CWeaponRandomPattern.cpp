#include "Weapons/CWeaponCombo.h"

#include "Commons/CYJJBlueprintLibrary.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ShapeComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Utilities/CLog.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

namespace RandomPatternAttachDetail
{
	void AttachActorRootToCharacterSocket(
		const ACWeaponRandomPattern* WeaponActor,
		USkeletalMeshComponent* CharMesh,
		const FName SocketName)
	{
		if (false == IsValid(WeaponActor) || false == IsValid(CharMesh) || SocketName.IsNone())
		{
			return;
		}

		USceneComponent* rootComp = WeaponActor->GetRootComponent();
		if (false == IsValid(rootComp))
		{
			CLog::Log(FString::Printf(
				TEXT("[WeaponRandomPattern] Root 없어 소켓 부착 생략 — %s Socket=%s"),
				*GetNameSafe(WeaponActor),
				*SocketName.ToString()));
			return;
		}

		rootComp->AttachToComponent(
			CharMesh,
			FAttachmentTransformRules::KeepRelativeTransform,
			SocketName);
	}
}

ACWeaponRandomPattern::ACWeaponRandomPattern()
{
	PrimaryActorTick.bCanEverTick = true;
	RandomSeed.Initialize(FMath::Rand());
}

void ACWeaponRandomPattern::BeginPlay()
{
	Super::BeginPlay();

	RandomSeed.Initialize(FMath::Rand());

	Collisions.Reset();
	GetComponents<UShapeComponent>(Collisions, true);
	OffCollisions();

	for (TObjectPtr<UShapeComponent>& ShapeWeak : Collisions)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		UPrimitiveComponent* PrimPtr = ShapePtr;
		PrimPtr->OnComponentBeginOverlap.AddDynamic(
			this,
			&ACWeaponRandomPattern::RandomPatternOnShapeBeginOverlap);
	}

	RandomPattern_ApplyHolsterAttachIfConfigured();
}

USkeletalMeshComponent* ACWeaponRandomPattern::RandomPattern_ResolveOwnerCharacterMesh() const
{
	if (false == IsValid(Character))
	{
		return nullptr;
	}

	const ACharacter* charActor = Cast<ACharacter>(Character);
	if (false == IsValid(charActor))
	{
		return nullptr;
	}

	return charActor->GetMesh();
}

void ACWeaponRandomPattern::RandomPattern_AttachActorRootToCharacterSocket(const FName InSocketName) const
{
	USkeletalMeshComponent* charMesh = RandomPattern_ResolveOwnerCharacterMesh();
	RandomPatternAttachDetail::AttachActorRootToCharacterSocket(this, charMesh, InSocketName);
}

void ACWeaponRandomPattern::RandomPattern_ApplyHolsterAttachIfConfigured() const
{
	if (HolsterAttachSocketName.IsNone())
	{
		return;
	}

	RandomPattern_AttachActorRootToCharacterSocket(HolsterAttachSocketName);
}

void ACWeaponRandomPattern::RandomPattern_ApplyHandAttachIfConfigured() const
{
	if (HandAttachSocketName.IsNone())
	{
		return;
	}

	RandomPattern_AttachActorRootToCharacterSocket(HandAttachSocketName);
}

void ACWeaponRandomPattern::Unequip_Implementation()
{
	RandomPattern_ApplyHolsterAttachIfConfigured();
	Super::Unequip_Implementation();
}

void ACWeaponRandomPattern::Begin_Equip_Implementation(const bool bMainOrSubWeapon)
{
	Super::Begin_Equip_Implementation(bMainOrSubWeapon);
	RandomPattern_ApplyHandAttachIfConfigured();
}

void ACWeaponRandomPattern::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : Collisions)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		UPrimitiveComponent* PrimPtr = ShapePtr;
		PrimPtr->OnComponentBeginOverlap.RemoveDynamic(
			this,
			&ACWeaponRandomPattern::RandomPatternOnShapeBeginOverlap);
	}

	Collisions.Reset();
	Super::EndPlay(EndPlayReason);
}

void ACWeaponRandomPattern::OnCollisions()
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : Collisions)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		ShapePtr->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void ACWeaponRandomPattern::OffCollisions()
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : Collisions)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		ShapePtr->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	Hitted.Reset();
}

void ACWeaponRandomPattern::EnableCombo()
{
	Enable = true;
}

void ACWeaponRandomPattern::DisableCombo()
{
	Enable = false;
}

void ACWeaponRandomPattern::Host_OnCollisions()
{
	OnCollisions();
}

void ACWeaponRandomPattern::Host_OffCollisions()
{
	OffCollisions();
}

void ACWeaponRandomPattern::Host_EnableCombo()
{
	EnableCombo();
}

void ACWeaponRandomPattern::Host_DisableCombo()
{
	DisableCombo();
}

void ACWeaponRandomPattern::PlayRandomActionMontage()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponRandomPattern] PlayRandomActionMontage(Character 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	if (DoActionDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponRandomPattern] PlayRandomActionMontage(DoActionDatas 비어 있음): %s"),
			*GetNameSafe(this)));
		return;
	}

	const int32 MaxInclusive =
		FMath::Min(RandomPatternMontageRandomMax, DoActionDatas.Num() - 1);
	if (MaxInclusive < 0)
	{
		return;
	}

	RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(RandomSeed, MaxInclusive);

	if (false == DoActionDatas.IsValidIndex(RandomIndex))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponRandomPattern] PlayRandomActionMontage(인덱스 무효): %s Index=%d Count=%d"),
			*GetNameSafe(this),
			RandomIndex,
			DoActionDatas.Num()));
		return;
	}

	const FDoActionData& Row = DoActionDatas[RandomIndex];
	if (IsValid(Row.Montage))
	{
		Character->PlayAnimMontage(Row.Montage, Row.PlayRate);
	}

	if (IsValid(Moving))
	{
		if (Row.bCanMove)
		{
			Moving->Move();
		}
		else
		{
			Moving->Stop();
		}
	}
}

bool ACWeaponRandomPattern::RandomPattern_TryRegisterHitTarget(ACCommonCharacter* InCharacter)
{
	if (false == IsValid(InCharacter))
	{
		return false;
	}

	TArray<ACCommonCharacter*> Scratch;
	Scratch.Reserve(Hitted.Num() + 1);
	for (TObjectPtr<ACCommonCharacter>& EntryWeak : Hitted)
	{
		ACCommonCharacter* EntryRaw = EntryWeak.Get();
		if (IsValid(EntryRaw))
		{
			Scratch.Add(EntryRaw);
		}
	}

	const bool bInserted = UCYJJBlueprintLibrary::TryAddUniqueCommonCharacter(Scratch, InCharacter);
	Hitted.Reset();
	for (ACCommonCharacter* EntryRaw : Scratch)
	{
		Hitted.Add(EntryRaw);
	}

	return bInserted;
}

void ACWeaponRandomPattern::OnBeginOverlap_Implementation(
	ACCommonCharacter* InOtherCharacter,
	const FVector& InHitPoint)
{
	(void)InHitPoint;

	if (false == IsValid(InOtherCharacter) || false == IsValid(Character))
	{
		return;
	}

	if (false == RandomPattern_TryRegisterHitTarget(InOtherCharacter))
	{
		return;
	}

	if (false == HitCommonDatas.IsValidIndex(RandomIndex))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponRandomPattern] OnBeginOverlap(HitCommonDatas 인덱스 없음): %s RandomIndex=%d Count=%d"),
			*GetNameSafe(this),
			RandomIndex,
			HitCommonDatas.Num()));
		return;
	}

	HitCommonDatas[RandomIndex].SendDamage(
		TWeakObjectPtr<ACCommonCharacter>(Character),
		this,
		InOtherCharacter);
}

void ACWeaponRandomPattern::RandomPatternOnShapeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	ACCommonCharacter* HitChar = Cast<ACCommonCharacter>(OtherActor);
	if (false == IsValid(HitChar) || false == IsValid(Character))
	{
		return;
	}

	if (true == UCYJJBlueprintLibrary::AreCharactersSameGroup(Character, HitChar))
	{
		return;
	}

	TArray<AActor*> ActorsIgnore;
	ActorsIgnore.Reserve(2);
	ActorsIgnore.Add(Character);

	FVector ImpactWorld =
		IsValid(OtherComp) ? OtherComp->K2_GetComponentLocation() : HitChar->GetActorLocation();

	const bool TraceOk = UCYJJBlueprintLibrary::TryGetHitPointBetweenPrimitives(
		OverlappedComponent,
		OtherComp,
		RandomPatternMeleeHitSphereRadius,
		ActorsIgnore,
		ImpactWorld);

	if (false == TraceOk)
	{
		if (true == IsValid(OtherComp))
		{
			ImpactWorld = OtherComp->K2_GetComponentLocation();
		}
		else
		{
			ImpactWorld = HitChar->GetActorLocation();
		}
	}

	OnBeginOverlap(HitChar, ImpactWorld);
}

void ACWeaponRandomPattern::Weapon_DoActionImpl(
	const CEAttackType InAttackType,
	const int32 InSkillIndex)
{
	(void)InSkillIndex;

	CancelDashes();

	if (DoActionDatas.Num() <= 0)
	{
		return;
	}

	if (false == Enable)
	{
		return;
	}

	DisableCombo();

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponRandomPattern] DoAction(State 없음): %s"),
			*GetNameSafe(this)));
		return;
	}

	const CEStateType CurMode = StateComp->GetCurMode();
	if (CurMode == CEStateType::Idle || CurMode == CEStateType::CombatHitted)
	{
		StateComp->SetAction();
		PlayRandomActionMontage();
		InAction = true;
		PrevAttackType = InAttackType;
	}
}

void ACWeaponRandomPattern::End_DoAction(const CEAttackType InAttackType)
{
	Super::End_DoAction(InAttackType);
	Enable = true;
}

void ACWeaponRandomPattern::End_Equip_Implementation(const bool bMainOrSubWeapon)
{
	Super::End_Equip_Implementation(bMainOrSubWeapon);
	EnableCombo();
}

ACWeaponRandomPatternSword::ACWeaponRandomPatternSword()
{
	HolsterAttachSocketName = FName(TEXT("Holster_Sword"));
	HandAttachSocketName = FName(TEXT("Hand_Sword"));
}

