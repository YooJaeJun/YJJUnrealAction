#include "Characters/Animals/CAnimalWeapon.h"

#include "Animation/AnimMontage.h"
#include "Commons/CYJJBlueprintLibrary.h"
#include "Utilities/CLog.h"
#include "Characters/Animals/CAnimal.h"
#include "Characters/CCommonCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Weapons/CWeaponStructures.h"
#include "Engine/DataTable.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

FName ACAnimalWeapon::AnimalWeaponAttachSocket()
{
	return FName(TEXT("Weapon"));
}

ACAnimalWeapon::ACAnimalWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRoot);
	DefaultSceneRoot->bVisualizeComponent = true;

	SphereOverlapForNavObstacle = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereOverlapForNavObstacle->SetupAttachment(DefaultSceneRoot);
	SphereOverlapForNavObstacle->SetSphereRadius(200.0f);
}

void ACAnimalWeapon::BeginPlay()
{
	Super::BeginPlay();

	Animal_RuntimeRefreshOwnerCharacterFromOverlapContext();

	if (false == IsValid(CharacterOwner))
	{
		CLog::Log(FString::Printf(
			TEXT("[AnimalWeapon] CharacterOwner 없음(EventGraph 소유 Animal_AI 캐스트 실패) — 부착·바인딩 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	Animal_RuntimeAttachSelfToAnimalMeshWeaponSocket();

	Animal_RuntimeCollectCollisionShapesMatchingBp();

	OffCollisions();

	Animal_RuntimeBindOverlapDelegates();

	Animal_LoadTablesAllRows_LoadHitDatas();

	Animal_LoadTablesAllRows_LoadDoActionDatas();
}

void ACAnimalWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Animal_RuntimeUnbindOverlapDelegates();

	Super::EndPlay(EndPlayReason);
}

void ACAnimalWeapon::OnCollisions()
{
	const int collisionCountLocal = Collisions.Num();
	for (int idxLocal = 0; idxLocal < collisionCountLocal; ++idxLocal)
	{
		UShapeComponent* shapePtrLocal = Collisions[idxLocal].Get();
		if (false == IsValid(shapePtrLocal))
		{
			continue;
		}

		UPrimitiveComponent* primResolved = shapePtrLocal;
		primResolved->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void ACAnimalWeapon::OffCollisions()
{
	const int collisionCountLocal = Collisions.Num();
	for (int idxLocal = 0; idxLocal < collisionCountLocal; ++idxLocal)
	{
		UShapeComponent* shapePtrLocal = Collisions[idxLocal].Get();
		if (false == IsValid(shapePtrLocal))
		{
			continue;
		}

		UPrimitiveComponent* primResolved = shapePtrLocal;
		primResolved->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	Hitted.Reset();
}

void ACAnimalWeapon::OnBeginOverlap_Implementation(ACCommonCharacter* OtherCharacter, FVector const& HitPoint)
{
	if (false == IsValid(OtherCharacter))
	{
		CLog::Log(FString::Printf(
			TEXT("[AnimalWeapon] OnBeginOverlap: OtherCharacter 무효 — 무시 — %s"),
			*GetNameSafe(this)));
		return;
	}

	if (false == Animal_HasAuthorityViaOwner())
	{
		// 레거시는 델리게이트 전체 플래그 경로지만, 무기별로는 로컬 피판정만 줄이려 이 경로에서는 조용히 둔다(소유 폰 무권위).
		return;
	}

	const int hittedUpperLocal = Hitted.Num();
	bool bSeenLocal = false;
	for (int hittedIdxLocal = 0; hittedIdxLocal < hittedUpperLocal; ++hittedIdxLocal)
	{
		TObjectPtr<ACCommonCharacter> existingMaybe = Hitted[hittedIdxLocal];
		if (existingMaybe.Get() == OtherCharacter)
		{
			bSeenLocal = true;
			break;
		}
	}

	if (true == bSeenLocal)
	{
		return;
	}

	const int appliedHitIxLocal = HitApplyRowIndex;
	if (false == HitDatas.IsValidIndex(appliedHitIxLocal))
	{
		CLog::Log(FString::Printf(
			TEXT("[AnimalWeapon] OnBeginOverlap: HitDatas[%d] 범위 밖(Num=%d) — %s"),
			appliedHitIxLocal,
			HitDatas.Num(),
			*GetNameSafe(this)));
		return;
	}

	if (false == IsValid(CharacterOwner))
	{
		CLog::Log(FString::Printf(
			TEXT("[AnimalWeapon] OnBeginOverlap: CharacterOwner 무효 — Damaged 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	Hitted.Add(TObjectPtr<ACCommonCharacter>(OtherCharacter));

	const FHitData& hitRowLocal = HitDatas[appliedHitIxLocal];

	// 레거시 `I_Damage::Damaged` — BP 에서 피격자 상태를 맞추기 위해 정보를 먼저 넣었다.
	OtherCharacter->SetDamagedInfo(
		CharacterOwner.Get(),
		this,
		hitRowLocal,
		HitPoint);

	hitRowLocal.SendDamage(
		TWeakObjectPtr<ACCommonCharacter>(CharacterOwner.Get()),
		TWeakObjectPtr<AActor>(this),
		TWeakObjectPtr<ACCommonCharacter>(OtherCharacter));

	if (IsValid(HitSoundAsset))
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSoundAsset, OtherCharacter->GetActorLocation());
	}
}

void ACAnimalWeapon::Animal_RuntimeRefreshOwnerCharacterFromOverlapContext()
{
	AActor* ownerResolved = GetOwner();
	ACAnimal* asAnimal = Cast<ACAnimal>(ownerResolved);
	if (false == IsValid(asAnimal))
	{
		CharacterOwner = nullptr;
		return;
	}

	CharacterOwner = asAnimal;
}

bool ACAnimalWeapon::Animal_HasAuthorityViaOwner() const
{
	const AActor* ownerResolved = GetOwner();
	if (false == IsValid(ownerResolved))
	{
		return false;
	}

	return ownerResolved->HasAuthority();
}

void ACAnimalWeapon::Animal_RuntimeAttachSelfToAnimalMeshWeaponSocket()
{
	if (false == IsValid(CharacterOwner))
	{
		return;
	}

	USkeletalMeshComponent* meshResolved = CharacterOwner->GetMesh();
	if (false == IsValid(meshResolved))
	{
		CLog::Log(FString::Printf(
			TEXT("[AnimalWeapon] Mesh 없음 — Weapon 소켓 부착 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	const FAttachmentTransformRules attachRules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepWorld,
		true);

	AttachToComponent(meshResolved, attachRules, AnimalWeaponAttachSocket());
}

void ACAnimalWeapon::Animal_RuntimeCollectCollisionShapesMatchingBp()
{
	Collisions.Reset();

	TArray<UShapeComponent*> scratchLocal;
	GetComponents<UShapeComponent>(scratchLocal);

	const int scannedCountLocal = scratchLocal.Num();
	Collisions.Reserve(scannedCountLocal);

	for (int scanIdxLocal = 0; scanIdxLocal < scannedCountLocal; ++scanIdxLocal)
	{
		UShapeComponent* shapeCandidate = scratchLocal[scanIdxLocal];
		if (false == IsValid(shapeCandidate))
		{
			continue;
		}

		Collisions.Add(shapeCandidate);
	}

	if (0 == Collisions.Num())
	{
		CLog::Log(FString::Printf(
			TEXT("[AnimalWeapon] ShapeComponent 검색 결과 0개 — 레거시 K2_GetComponentsByClass 과 불일치 여부 확인 — %s"),
			*GetNameSafe(this)));
	}
}

void ACAnimalWeapon::Animal_RuntimeBindOverlapDelegates()
{
	const int collisionUpperLocal = Collisions.Num();
	for (int collIdxLocal = 0; collIdxLocal < collisionUpperLocal; ++collIdxLocal)
	{
		UShapeComponent* shapePtrLocal = Collisions[collIdxLocal].Get();
		if (false == IsValid(shapePtrLocal))
		{
			continue;
		}

		UPrimitiveComponent* primResolved = shapePtrLocal;
		primResolved->OnComponentBeginOverlap.RemoveDynamic(this, &ACAnimalWeapon::Animal_OnShapeBeginOverlap);
		primResolved->OnComponentBeginOverlap.AddDynamic(this, &ACAnimalWeapon::Animal_OnShapeBeginOverlap);
	}
}

void ACAnimalWeapon::Animal_RuntimeUnbindOverlapDelegates()
{
	const int collisionUpperLocal = Collisions.Num();
	for (int collIdxLocal = 0; collIdxLocal < collisionUpperLocal; ++collIdxLocal)
	{
		UShapeComponent* shapePtrLocal = Collisions[collIdxLocal].Get();
		if (false == IsValid(shapePtrLocal))
		{
			continue;
		}

		UPrimitiveComponent* primResolved = shapePtrLocal;
		primResolved->OnComponentBeginOverlap.RemoveDynamic(this, &ACAnimalWeapon::Animal_OnShapeBeginOverlap);
	}
}

void ACAnimalWeapon::Animal_LoadTablesAllRows_LoadHitDatas()
{
	HitDatas.Reset();

	if (false == IsValid(HitDataTable))
	{
		return;
	}

	const UScriptStruct* const rowStructLocal = HitDataTable->GetRowStruct();
	const UScriptStruct* const hitStructLocal = FHitData::StaticStruct();
	if ((false == IsValid(rowStructLocal)) || (rowStructLocal != hitStructLocal))
	{
		CLog::Log(FString::Printf(
			TEXT("[AnimalWeapon] HitDataTable RowStruct 가 FHitData 가 아니거나 미설정. 에디터에서 DT 행 타입을 /Script/YJJActionCppUE5.HitData 로 지정(구 YJJActionCpp 레퍼런스 제거). DT=%s 현재 Struct=%s"),
			*HitDataTable->GetPathName(),
			IsValid(rowStructLocal) ? *rowStructLocal->GetStructPathName().ToString() : TEXT("(null)")));
		return;
	}

	const TArray<FName> rowNamesResolved = HitDataTable->GetRowNames();
	const int upperLocal = rowNamesResolved.Num();

	for (int rowIdxLocal = 0; rowIdxLocal < upperLocal; ++rowIdxLocal)
	{
		const FName rowNameResolved = rowNamesResolved[rowIdxLocal];
		const FHitData* rowResolved =
			HitDataTable->FindRow<FHitData>(rowNameResolved, TEXT("AnimalWeaponHit_Load"));

		if (nullptr == rowResolved)
		{
			continue;
		}

		HitDatas.Add(*rowResolved);
	}
}

void ACAnimalWeapon::Animal_LoadTablesAllRows_LoadDoActionDatas()
{
	DoActionDatas.Reset();

	if (false == IsValid(DoActionDataTable))
	{
		return;
	}

	const UScriptStruct* const rowStructLocal = DoActionDataTable->GetRowStruct();
	const UScriptStruct* const doActStructLocal = FDoActionData::StaticStruct();
	if ((false == IsValid(rowStructLocal)) || (rowStructLocal != doActStructLocal))
	{
		CLog::Log(FString::Printf(
			TEXT("[AnimalWeapon] DoAction Table RowStruct 가 FDoActionData 가 아니거나 미설정. 에디터에서 YJJActionCppUE5.FDoActionData 로 지정. DT=%s 현재 Struct=%s"),
			*DoActionDataTable->GetPathName(),
			IsValid(rowStructLocal) ? *rowStructLocal->GetStructPathName().ToString() : TEXT("(null)")));
		return;
	}

	const TArray<FName> rowNamesResolved = DoActionDataTable->GetRowNames();
	const int upperLocal = rowNamesResolved.Num();

	for (int rowIdxLocal = 0; rowIdxLocal < upperLocal; ++rowIdxLocal)
	{
		const FName rowNameResolved = rowNamesResolved[rowIdxLocal];
		const FDoActionData* rowResolved =
			DoActionDataTable->FindRow<FDoActionData>(rowNameResolved, TEXT("AnimalWeaponDoAction_Load"));

		if (nullptr == rowResolved)
		{
			continue;
		}

		DoActionDatas.Add(*rowResolved);
	}
}

void ACAnimalWeapon::Animal_OnShapeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)bFromSweep;

	(void)OtherBodyIndex;
	(void)SweepResult;

	if (false == IsValid(OtherActor))
	{
		return;
	}

	if (OtherActor == this)
	{
		return;
	}

	if (OtherActor == CharacterOwner)
	{
		return;
	}

	ACCommonCharacter* hitCharacterResolved = Cast<ACCommonCharacter>(OtherActor);
	if (false == IsValid(hitCharacterResolved))
	{
		return;
	}

	if (false == IsValid(CharacterOwner))
	{
		return;
	}

	if (true ==
		UCYJJBlueprintLibrary::AreCharactersSameGroup(CharacterOwner.Get(), hitCharacterResolved))
	{
		// 레거시 IsSameGroup==true 브랜치 — 아군이면 근거리 판정·피격을 건너뜀(Is Not Valid 브랜치는 비어 있음).
		return;
	}

	FVector tracedHitPointResolved = FVector::ZeroVector;

	if (true == Animal_TraceHitPointBetween(OverlappedComponent, OtherComp, tracedHitPointResolved))
	{
		OnBeginOverlap(hitCharacterResolved, tracedHitPointResolved);
		return;
	}

	OnBeginOverlap(hitCharacterResolved, OtherComp ? OtherComp->K2_GetComponentLocation() : hitCharacterResolved->GetActorLocation());
}

bool ACAnimalWeapon::Animal_TraceHitPointBetween(
	UPrimitiveComponent* OverlappedPrim,
	UPrimitiveComponent* OtherPrim,
	FVector& OutHitPointWorld) const
{
	if (false == IsValid(CharacterOwner))
	{
		return false;
	}

	TArray<AActor*> ignoreActorsLocal;
	ignoreActorsLocal.Reserve(2);
	ignoreActorsLocal.Add(CharacterOwner.Get());

	bool traceOkResolved = UCYJJBlueprintLibrary::TryGetHitPointBetweenPrimitives(
		OverlappedPrim,
		OtherPrim,
		BetweenPrimitivesHitSphereRadius,
		ignoreActorsLocal,
		OutHitPointWorld);

	return traceOkResolved;
}

void ACAnimalWeapon::Begin_DoAction()
{
}

void ACAnimalWeapon::End_DoAction()
{
	InAction = false;
}

void ACAnimalWeapon::DoAction(FVector const& InTargetLocation)
{
	if (false == Animal_HasAuthorityViaOwner())
	{
		return;
	}

	if (false == IsValid(CharacterOwner))
	{
		CLog::Log(FString::Printf(
			TEXT("[AnimalWeapon] DoAction: CharacterOwner 무효 — 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	InAction = true;

	// 블프 DoAction 순서 첫 줄: 타깃을 바라보도록 즉시 회전(FindLookAtRotation → SetActorRotation).
	const FVector attackerLocationLocal = CharacterOwner->GetActorLocation();
	const FRotator faceRotationLocal =
		UKismetMathLibrary::FindLookAtRotation(attackerLocationLocal, InTargetLocation);

	CharacterOwner->SetActorRotation(faceRotationLocal, ETeleportType::None);

	TArray<TObjectPtr<UAnimMontage>>& montagesLocalRef = CharacterOwner->AttackAnim;
	const int montageCountLocal = montagesLocalRef.Num();
	if (montageCountLocal > 0)
	{
		const int randomPickIdxLocal =
			FMath::RandRange(0, montageCountLocal - 1);
		UAnimMontage* pickedMontageLocal = montagesLocalRef[randomPickIdxLocal].Get();
		if (IsValid(pickedMontageLocal))
		{
			CharacterOwner->PlayAnimMontage(pickedMontageLocal, 1.0f, NAME_None);
		}
	}

	if (IsValid(ActionSoundAsset))
	{
		const FVector weaponLocationLocal = GetActorLocation();
		UGameplayStatics::PlaySoundAtLocation(this, ActionSoundAsset, weaponLocationLocal);
	}
}
