#include "Weapons/Bow/CWeaponBow.h"

#include "Characters/CCommonCharacter.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Weapons/Bow/CSkillAim.h"
#include "Weapons/CSkillArrow.h"

#include "Utilities/CLog.h"
#include "Utilities/YJJHelpers.h"

ACWeaponBow::ACWeaponBow()
{
	BowPoseableMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PoseableMesh"));
	BowPoseableMesh->SetupAttachment(DefaultSceneRoot);
	BowPoseableMesh->SetRelativeTransform(FTransform::Identity);

	BowPoseableMesh->PrimaryComponentTick.bCanEverTick = true;
	BowPoseableMesh->PrimaryComponentTick.bStartWithTickEnabled = true;
	BowPoseableMesh->PrimaryComponentTick.TickInterval = 0.f;

	TObjectPtr<USkeletalMesh> BowSkinnedAssetRef = nullptr;
	YJJHelpers::GetAsset<USkeletalMesh>(
		&BowSkinnedAssetRef,
		FString(
			TEXT("/Script/Engine.SkeletalMesh'/Game/Assets/Character/Weapons/ElvenBow/SK_ElvenBow.SK_ElvenBow'")));
	if (IsValid(BowSkinnedAssetRef))
	{
		BowPoseableMesh->SetSkinnedAsset(BowSkinnedAssetRef.Get());
	}

	TObjectPtr<UMaterialInterface> StraightSwordMat = nullptr;
	YJJHelpers::GetAsset<UMaterialInterface>(
		&StraightSwordMat,
		FString(TEXT("Material'/Game/Assets/Character/Weapons/M_Straight_Sword.M_Straight_Sword'")));
	if (IsValid(StraightSwordMat))
	{
		BowPoseableMesh->SetMaterial(0, StraightSwordMat.Get());
	}
}

void ACWeaponBow::Equip_Implementation()
{
	// 레거시 Weapon_Bow:Equip — 활 장착 시 캐릭터 스켈레탈 메시 트레이스가 활과 겹치지 않도록 끈다.
	Bow_SetCharacterMeshCollision(ECollisionEnabled::NoCollision);
	Super::Equip_Implementation();
}

void ACWeaponBow::Pressed_Implementation()
{
	Super::Pressed_Implementation();

	if (IsValid(Aim))
	{
		Aim->Pressed();
	}
}

void ACWeaponBow::Released_Implementation()
{
	if (IsValid(Aim))
	{
		Aim->Released();
	}

	Super::Released_Implementation();
}

void ACWeaponBow::BeginPlay()
{
	Super::BeginPlay();

	Bow_AttachWeaponToCharacterSocket(HolsterSocketName);
	RefreshBowStringOrigin_ComponentSpace();
}

void ACWeaponBow::Bow_SetCharacterMeshCollision(const ECollisionEnabled::Type NewCollision)
{
	if (false == IsValid(Character))
	{
		return;
	}

	ACharacter* CharActor = Cast<ACharacter>(Character);
	if (false == IsValid(CharActor))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponBow] 메시 충돌 갱신 생략 — Character 가 ACharacter 가 아닙니다 Bow=%s"),
			*GetNameSafe(this)));
		return;
	}

	USkeletalMeshComponent* MeshComp = CharActor->GetMesh();
	if (false == IsValid(MeshComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponBow] 메시 충돌 갱신 생략 — Mesh 없음 Character=%s"), *GetNameSafe(Character)));
		return;
	}

	MeshComp->SetCollisionEnabled(NewCollision);
}

void ACWeaponBow::Bow_AttachWeaponToCharacterSocket(const FName& SocketName, const bool bUnequipHolsterRules)
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponBow] 소켓 %s 에 붙일 수 없음 — Character 없음 Actor=%s"),
			*SocketName.ToString(),
			*GetNameSafe(this)));
		return;
	}

	ACharacter* CharActor = Cast<ACharacter>(Character);
	if (false == IsValid(CharActor))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponBow] 소켓 %s 에 붙일 수 없음 — Character 가 ACharacter 가 아닙니다 Actor=%s"),
			*SocketName.ToString(),
			*GetNameSafe(this)));
		return;
	}

	USkeletalMeshComponent* MeshComp = CharActor->GetMesh();
	if (false == IsValid(MeshComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponBow] 소켓 %s 에 붙일 수 없음 — Mesh 없음 Actor=%s"),
			*SocketName.ToString(),
			*GetNameSafe(this)));
		return;
	}

	FAttachmentTransformRules Rules = FAttachmentTransformRules::KeepRelativeTransform;
	if (bUnequipHolsterRules)
	{
		Rules = FAttachmentTransformRules(
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepWorld,
			true);
	}

	AttachToComponent(MeshComp, Rules, SocketName);
}

void ACWeaponBow::RefreshBowStringOrigin_ComponentSpace()
{
	if (false == IsValid(BowPoseableMesh))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponBow] 활줄 기준좌표 갱신 생략 — Poseable 없음 Actor=%s"), *GetNameSafe(this)));
		return;
	}

	OriginLocation = BowPoseableMesh->GetBoneLocationByName(StringName, EBoneSpaces::ComponentSpace);
}

void ACWeaponBow::Bow_ResetStringBoneToOrigin_ComponentSpace()
{
	if (false == IsValid(BowPoseableMesh))
	{
		return;
	}

	BowPoseableMesh->SetBoneLocationByName(StringName, OriginLocation, EBoneSpaces::ComponentSpace);
}

void ACWeaponBow::Bow_DestroyAimActorIfValid()
{
	if (false == IsValid(Aim))
	{
		return;
	}

	Aim->Destroy();
	Aim = nullptr;
}

void ACWeaponBow::Bow_SpawnAimActorOwnedByBow()
{
	if (false == IsValid(AimSpawnClass))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponBow] AimSpawnClass 미설정 — Skill_Aim BP 클래스를 Bow 클래스 디폴트에 지정해야 합니다 Actor=%s"),
			*GetNameSafe(this)));
		return;
	}

	UWorld* World = GetWorld();
	if (false == IsValid(World))
	{
		return;
	}

	if (World->bIsTearingDown)
	{
		return;
	}

	if (IsValid(Aim))
	{
		Bow_DestroyAimActorIfValid();
	}

	FActorSpawnParameters Params;
	Params.Owner = this;

	ACSkillAim* SpawnedAim =
		World->SpawnActor<ACSkillAim>(AimSpawnClass, FTransform::Identity, Params);
	if (false == IsValid(SpawnedAim))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponBow] SkillAim 스폰 실패 — Class=%s Bow=%s"),
			*GetNameSafe(AimSpawnClass.Get()),
			*GetNameSafe(this)));
		return;
	}

	Aim = SpawnedAim;
}

void ACWeaponBow::Bow_DestroyArrowsAttachedToCharacter()
{
	const int32 NumArrows = Arrows.Num();
	for (int32 Index = NumArrows - 1; Index >= 0; --Index)
	{
		ACSkillArrow* ArrowActor = Arrows[Index].Get();
		if (false == IsValid(ArrowActor))
		{
			Arrows.RemoveAt(Index);
			continue;
		}

		if (false == IsValid(ArrowActor->GetAttachParentActor()))
		{
			continue;
		}

		ArrowActor->Destroy();
	}

	Arrow = nullptr;
}

void ACWeaponBow::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (false == InEquip)
	{
		return;
	}

	if (false == AttachBowString)
	{
		return;
	}

	if (false == IsValid(Character) || false == IsValid(BowPoseableMesh))
	{
		return;
	}

	ACharacter* CharActor = Cast<ACharacter>(Character);
	if (false == IsValid(CharActor))
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = CharActor->GetMesh();
	if (false == IsValid(MeshComp))
	{
		return;
	}

	const FVector PullHandWorld = MeshComp->GetSocketLocation(PullHandSocketName);
	BowPoseableMesh->SetBoneLocationByName(StringName, PullHandWorld, EBoneSpaces::WorldSpace);

	(void)DeltaSeconds;
}

void ACWeaponBow::Begin_Equip_Implementation(bool bMainOrSubWeapon)
{
	Super::Begin_Equip_Implementation(bMainOrSubWeapon);

	(void)bMainOrSubWeapon;
}

void ACWeaponBow::End_Equip_Implementation(bool bMainOrSubWeapon)
{
	Super::End_Equip_Implementation(bMainOrSubWeapon);

	if (false == bMainOrSubWeapon)
	{
		return;
	}

	CreateArrow();
	Bow_SpawnAimActorOwnedByBow();
	Bow_AttachWeaponToCharacterSocket(HandLeftSocketName);
}

void ACWeaponBow::Begin_Unequip_Implementation(bool bMainOrSubWeapon)
{
	Super::Begin_Unequip_Implementation(bMainOrSubWeapon);

	if (false == bMainOrSubWeapon)
	{
		return;
	}

	Bow_SetCharacterMeshCollision(ECollisionEnabled::QueryOnly);
	Bow_DestroyAimActorIfValid();
	Bow_ResetStringBoneToOrigin_ComponentSpace();
}

void ACWeaponBow::End_Unequip_Implementation(bool bMainOrSubWeapon)
{
	Super::End_Unequip_Implementation(bMainOrSubWeapon);

	if (false == bMainOrSubWeapon)
	{
		return;
	}

	Bow_DestroyArrowsAttachedToCharacter();
	Arrows.Reset();

	Bow_AttachWeaponToCharacterSocket(HolsterSocketName, true);

	AttachBowString = true;
	RefreshBowStringOrigin_ComponentSpace();
}

void ACWeaponBow::WeaponBow_OnArrowDestroyed(UObject* InArrowRaw)
{
	ACSkillArrow* InArrow = Cast<ACSkillArrow>(InArrowRaw);
	if (false == IsValid(InArrow))
	{
		return;
	}

	Arrows.Remove(InArrow);
	if (Arrow == InArrow)
	{
		Arrow = nullptr;
	}
}

void ACWeaponBow::CreateArrow()
{
	UWorld* World = GetWorld();
	if (false == IsValid(World))
	{
		CLog::Log(FString::Printf(TEXT("[WeaponBow] CreateArrow: World 없음 — %s"), *GetNameSafe(this)));
		return;
	}

	if (World->bIsTearingDown)
	{
		return;
	}

	UClass* ClassToSpawn = SkillArrowSpawnClass.Get();
	if (false == IsValid(ClassToSpawn))
	{
		ClassToSpawn = ACSkillArrow::StaticClass();
	}

	FActorSpawnParameters Params;
	Params.Owner = this;

	const FRotator SpawnRot = FRotator::ZeroRotator;
	const FVector SpawnLoc = FVector::ZeroVector;

	ACSkillArrow* NewArrow = World->SpawnActor<ACSkillArrow>(
		ClassToSpawn,
		FTransform(SpawnRot, SpawnLoc),
		Params);
	if (false == IsValid(NewArrow))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponBow] CreateArrow 스폰 실패 — Class=%s Owner=%s"),
			*GetNameSafe(ClassToSpawn),
			*GetNameSafe(this)));
		return;
	}

	NewArrow->OnArrowDestroied.AddDynamic(this, &ACWeaponBow::WeaponBow_OnArrowDestroyed);

	Arrow = NewArrow;
	Arrows.Add(NewArrow);

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponBow] 화살 스폰됐으나 캐릭터 미연결 — 소켓 부착 생략 Bow=%s"), *GetNameSafe(this)));
		return;
	}

	ACharacter* CharActor = Cast<ACharacter>(Character);
	if (false == IsValid(CharActor))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponBow] 화살 부착 생략 — Character 가 ACharacter 가 아닙니다 Bow=%s"),
			*GetNameSafe(this)));
		return;
	}

	USkeletalMeshComponent* MeshComp = CharActor->GetMesh();
	if (false == IsValid(MeshComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponBow] 화살 부착 생략 — Mesh 없음 Character=%s"), *GetNameSafe(Character)));
		return;
	}

	NewArrow->AttachToComponent(MeshComp, FAttachmentTransformRules::KeepRelativeTransform, ArrowAttachSocketName);

	AttachBowString = true;
	RefreshBowStringOrigin_ComponentSpace();
}

void ACWeaponBow::Begin_DoAction_Implementation(CEAttackType InAttackType)
{
	Super::Begin_DoAction_Implementation(InAttackType);

	(void)InAttackType;

	const int32 NumArrows = Arrows.Num();
	ACSkillArrow* ArrowToShoot = nullptr;

	for (int32 Index = 0; Index < NumArrows; ++Index)
	{
		if (false == IsValid(Arrows[Index]))
		{
			continue;
		}

		if (false == IsValid(Arrows[Index]->GetAttachParentActor()))
		{
			continue;
		}

		ArrowToShoot = Arrows[Index].Get();
		break;
	}

	if (false == IsValid(ArrowToShoot))
	{
		// Tick 과 동일하게 유효한 부모 소켓이 있는 화살이 없을 수 있음 — 스팸 방지 로그 없음.
		return;
	}

	ArrowToShoot->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	FVector ShootDir = FVector::ForwardVector;
	if (IsValid(Character))
	{
		if (IsValid(Character->CurController))
		{
			ShootDir = UKismetMathLibrary::GetForwardVector(Character->CurController->GetControlRotation());
		}
		else
		{
			ShootDir = FRotationMatrix(Character->GetControlRotation()).GetUnitAxis(EAxis::X);
		}
	}

	ArrowToShoot->Shoot(ShootDir);
	AttachBowString = false;

	if (IsValid(BowPoseableMesh))
	{
		BowPoseableMesh->SetBoneLocationByName(StringName, OriginLocation, EBoneSpaces::ComponentSpace);
	}
}

void ACWeaponBow::End_DoAction(CEAttackType InAttackType)
{
	Super::End_DoAction(InAttackType);

	CreateArrow();
}
