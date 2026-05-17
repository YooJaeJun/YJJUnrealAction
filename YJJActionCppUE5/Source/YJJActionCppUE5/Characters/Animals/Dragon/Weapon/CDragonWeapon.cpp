#include "Characters/Animals/Dragon/Weapon/CDragonWeapon.h"

#include "Characters/Animals/Dragon/CDragon.h"
#include "Characters/CCommonCharacter.h"
#include "Commons/CEnums.h"
#include "Commons/CYJJBlueprintLibrary.h"
#include "Components/CMovementComponent.h"
#include "Components/CFlyComponent.h"
#include "Components/CStateComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Global.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Utilities/CLog.h"
#include "Weapons/CWeaponStructures.h"

namespace
{
	// 레거시 `/Game/.../FireAir/EFlyState` — 열거자 이름(NewEnumerator*) 과 동일한 바이트를 가진다고 가정한다(자산 재정렬 시 인스펙터에서 맞춤).
	constexpr uint8 GDragonBossEFly_None = 0;
	constexpr uint8 GDragonBossEFly_Jump = 1;
	constexpr uint8 GDragonBossEFly_Fly = 2;
	constexpr uint8 GDragonBossEFly_Land = 3;
	constexpr uint8 GDragonBossEFly_Wait = 5;
	constexpr uint8 GDragonBossEFly_Set = 6;
}

ACDragonWeapon::ACDragonWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	DragonWeaponNavOverlapExtra = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent_NavOverlapExtra"));

	DragonWeaponNavOverlapExtra->SetupAttachment(DefaultSceneRoot);
	DragonWeaponNavOverlapExtra->SetSphereRadius(200.0f);
	DragonWeaponNavOverlapExtra->SetHiddenInGame(true);
	// 블프 레이아웃의 두 번째 스피어 근사 — 충돌 이중 적용을 피해서 꺼둠(내비만 쓸 경우 에디터에서 충돌 프로필을 맞춘다).
	DragonWeaponNavOverlapExtra->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SkillWaitTime = TArray<double>{ 5.0, 8.0, 8.0, 15.0, 4.0, 12.0 };

	YJJHelpers::GetClass<AActor>(
		&FireFlySkillActorClass,
		TEXT("/Script/Engine.Blueprint'/Game/Character/Animals/Dragon/Weapon/FireAir/Skill_Dragon_FireAir.Skill_Dragon_FireAir_C'"));
	YJJHelpers::GetClass<AActor>(
		&SplineClass,
		TEXT("/Script/Engine.Blueprint'/Game/Character/Animals/Dragon/Weapon/FireAir/BP_Spline.BP_Spline_C'"));
}

void ACDragonWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (false == HasAuthority())
	{
		return;
	}
	Dragon_TickRotate(DeltaSeconds);
	Dragon_TickFireFlyBossGraph(DeltaSeconds);
	Dragon_TickFlyThunderBossGraph(DeltaSeconds);
}

void ACDragonWeapon::Dragon_TickRotate(float const DeltaSeconds)
{
	if (false == HasAuthority())
	{
		// 회전 재현은 캐릭터·폰 기본 복제에 맡김 — 클라 틱 SetActorRotation 는 권위 교정 깜빡임을 유발할 수 있음.
		return;
	}

	ACDragon* const dragonScratch = Dragon_ResolveOwningDragon();
	ACCommonCharacter* const playerScratch = Player.Get();

	if ((false == IsValid(dragonScratch)) || (false == IsValid(playerScratch)))
	{
		// 드래곤·플레이어 레퍼런스 비워진 구간에서는 조용히 패스(블프 변수 미세팅·전투 밖 장면 등).
		return;
	}

	if (false == dragonScratch->IsIdle())
	{
		// 블프 Switch 는 Idle 핀만 다음 분기 실행 — 상태 전환별로 회전 호출량을 줄임(로그·틱 과다 금지).
		return;
	}

	if (true == UCYJJBlueprintLibrary::AreCharactersSameGroup(dragonScratch, playerScratch))
	{
		return;
	}

	const FVector dragonLocation = dragonScratch->GetActorLocation();
	const FVector playerLocation = playerScratch->GetActorLocation();
	const FRotator lookTowardPlayerRotation = UKismetMathLibrary::FindLookAtRotation(dragonLocation, playerLocation);

	const FRotator currentRotation = dragonScratch->GetActorRotation();
	FRotator targetRotation = currentRotation;
	targetRotation.Roll = lookTowardPlayerRotation.Roll;
	targetRotation.Pitch = 0.0f;
	targetRotation.Yaw = lookTowardPlayerRotation.Yaw;

	const FRotator newRotation = UKismetMathLibrary::RInterpTo(
		currentRotation,
		targetRotation,
		DeltaSeconds,
		RotateTowardPlayerInterpSpeed);

	dragonScratch->SetActorRotation(newRotation);
}

void ACDragonWeapon::BeginPlay()
{
	Super::BeginPlay();

	ACDragon* const dragonScratch = Cast<ACDragon>(CharacterOwner.Get());

	if (false == IsValid(dragonScratch))
	{
		// 비드래곤 소유 무기까지 공용 레퍼런스로 쓰이면 드물게 발생 — 디버깅 가능할 때만 남김.
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] CharacterOwner 가 ACDragon 아님 — Dragon 자동 채우기 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	if (false == IsValid(Dragon.Get()))
	{
		// 레거시 Dragon 변수 에디터 비어 있었을 때 CharacterOwner 에서 채워 동일 변수 그래프를 유지한다.
		Dragon = dragonScratch;
	}

	Dragon_TryDiscoverPlayerTaggedActorIntoProperty();
	Dragon_TryEnsureSplineActorSpawnedAtDragon(dragonScratch);
}

void ACDragonWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* const worldScratch = GetWorld();
	if (IsValid(worldScratch))
	{
		Dragon_ClearFireFlyThunderLatentTimers();
		worldScratch->GetTimerManager().ClearTimer(DragonSkillPhaseEndTimerHandle);
		worldScratch->GetTimerManager().ClearTimer(DragonMeteorSpawnTimerHandle);
	}

	Dragon_ResetFireFlyThunderRuntimeStateExceptSplineActor(true);

	Dragon_RuntimeTeardownSpawnedSkillActors();

	ACDragon* const dragonScratch = Dragon_ResolveOwningDragon();
	Dragon_RuntimeRestoreNeckAfterFireFlyIfNeeded(dragonScratch);

	if (IsValid(dragonScratch))
	{
		// 액터 파괴 중에도 카메라·이동 잠금이 남지 않게 정리한다(스킬 타이머가 끊긴 케이스 포함).
		Dragon_ApplySkillPostlude(dragonScratch);
	}
	else
	{
		Hitted.Empty();
		InAction = false;
	}

	Super::EndPlay(EndPlayReason);
}

void ACDragonWeapon::Skill_Implementation(uint8 const InSkillType)
{
	if (false == HasAuthority())
	{
		// Skill 은 서버 권위만 — 클라는 드래곤 Server RPC 를 통해서만 호출되어야 한다.
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill 은 서버에서만 실행 — 건너뜀 — 값=%u, 액터=%s"),
			static_cast<uint32>(InSkillType),
			*GetNameSafe(this)));
		return;
	}

	const uint8 skillByteScratch = static_cast<uint8>(InSkillType);
	if (skillByteScratch >= static_cast<uint8>(CEDragonBossSkillType::Max))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill 바이트가 EDragonBossSkillType 범위 밖임 — 값=%u, 액터=%s"),
			static_cast<uint32>(InSkillType),
			*GetNameSafe(this)));
		return;
	}

	const CEDragonBossSkillType skillScratch = static_cast<CEDragonBossSkillType>(InSkillType);
	ACDragon* const dragonResolvedScratch = Dragon_ResolveOwningDragon();
	if (false == IsValid(dragonResolvedScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill 불가 — Dragon 을 CharacterOwner/Dragon 슬롯에서 찾을 수 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	Dragon_AbortSkillInFlight(dragonResolvedScratch);
	Dragon_ApplySkillPrelude(dragonResolvedScratch);
	InAction = true;

	switch (skillScratch)
	{
	case CEDragonBossSkillType::Roar:
		Skill_1();
		break;
	case CEDragonBossSkillType::Fire:
		Skill_2();
		break;
	case CEDragonBossSkillType::Meteor:
		Skill_3();
		break;
	case CEDragonBossSkillType::FireFly:
		Skill_4();
		break;
	case CEDragonBossSkillType::Thunder:
		Skill_5();
		break;
	case CEDragonBossSkillType::Fly:
		Skill_6();
		break;
	}
}

void ACDragonWeapon::Begin_Skill(uint8 const InSkillType)
{
	Skill(InSkillType);
}

void ACDragonWeapon::End_Skill(uint8 const InSkillType)
{
	if (false == HasAuthority())
	{
		CLog::Log(FString::Printf(TEXT("[DragonWeapon] End_Skill 은 서버에서만 실행 — 건너뜀 — 값=%u, 액터=%s"),
			static_cast<uint32>(InSkillType), *GetNameSafe(this)));
		return;
	}

	const uint8 skillByteScratch = static_cast<uint8>(InSkillType);
	if (skillByteScratch >= static_cast<uint8>(CEDragonBossSkillType::Max))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] End_Skill 바이트가 EDragonBossSkillType 범위 밖 — 값=%u, 액터=%s"),
			static_cast<uint32>(InSkillType),
			*GetNameSafe(this)));
		return;
	}

	// 레거시 `End_Skill(InSkillType)` — 바이트는 검증만 하고 현재 페이즈 정리 경로와 동등하게 처리(InSkillType 은 블프·노티 시그니처 보존).

	UWorld* const worldScratchForTimerClear = GetWorld();
	if (IsValid(worldScratchForTimerClear))
	{
		worldScratchForTimerClear->GetTimerManager().ClearTimer(DragonSkillPhaseEndTimerHandle);
	}

	Dragon_OnSkillPhaseEndTimerExpired();
}

void ACDragonWeapon::Skill_1_Implementation()
{
	ACDragon* const dragonScratch = Dragon_ResolveOwningDragon();
	if (false == IsValid(dragonScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_1(Roar) Dragon 없음 — 타이머만으로 포스트 정리 — %s"),
			*GetNameSafe(this)));
		Skill_SchedulePhaseEnd(1.0f);
		return;
	}

	Hitted.Empty();

	if (true == DoActionDatas.IsValidIndex(RoarIndex))
	{
		const FDoActionData& rowScratch = DoActionDatas[RoarIndex];
		if (IsValid(rowScratch.Montage))
		{
			dragonScratch->PlayAnimMontage(rowScratch.Montage, rowScratch.PlayRate);
		}
		if (IsValid(rowScratch.Sound))
		{
			rowScratch.PlaySoundWave(TWeakObjectPtr<ACCommonCharacter>(dragonScratch));
		}
		const TWeakObjectPtr<UWorld> worldWeak = dragonScratch->GetWorld();
		if ((true == worldWeak.IsValid()) && (IsValid(rowScratch.Effect)))
		{
			rowScratch.PlayEffect(worldWeak, dragonScratch->GetActorLocation(), dragonScratch->GetActorRotation());
		}
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_1(Roar) DoActionDatas 인덱스 없음 — RoarIndex=%d, Num=%d — %s"),
			RoarIndex,
			DoActionDatas.Num(),
			*GetNameSafe(this)));
	}

	if (false == HitDatas.IsValidIndex(RoarIndex))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_1(Roar) HitDatas 인덱스 없음 — RoarIndex=%d, Num=%d — %s"),
			RoarIndex,
			HitDatas.Num(),
			*GetNameSafe(this)));
		const FDoActionData* const montageRowScratch =
			DoActionDatas.IsValidIndex(RoarIndex) ? &DoActionDatas[RoarIndex] : nullptr;
		const float scheduleScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
			montageRowScratch,
			SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::Roar),
			5.0f,
			nullptr,
			1.0f);
		Skill_SchedulePhaseEnd(scheduleScratch);
		return;
	}

	const FVector sphereOriginScratch = dragonScratch->GetActorLocation();
	TArray<TEnumAsByte<EObjectTypeQuery>> objectQueriesScratch;
	objectQueriesScratch.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ignoreActorsScratch;
	ignoreActorsScratch.Add(dragonScratch);
	ignoreActorsScratch.Add(this);

	TArray<FHitResult> hitResultsScratch;
	const bool tracedScratch = UKismetSystemLibrary::SphereTraceMultiForObjects(
		this,
		sphereOriginScratch,
		sphereOriginScratch,
		static_cast<float>(RoarRadius),
		objectQueriesScratch,
		false,
		ignoreActorsScratch,
		EDrawDebugTrace::None,
		hitResultsScratch,
		true);

	if ((true == tracedScratch) && (hitResultsScratch.Num() > 0))
	{
		for (int32 hitIndexScratch = 0; hitIndexScratch < hitResultsScratch.Num(); ++hitIndexScratch)
		{
			FHitResult const& hitScratch = hitResultsScratch[hitIndexScratch];
			ACCommonCharacter* const hitCharacterScratch = Cast<ACCommonCharacter>(hitScratch.GetActor());
			if (false == IsValid(hitCharacterScratch))
			{
				continue;
			}

			bool duplicatedScratch = false;
			for (int32 hittedIndexScratch = 0; hittedIndexScratch < Hitted.Num(); ++hittedIndexScratch)
			{
				if (Hitted[hittedIndexScratch].Get() == hitCharacterScratch)
				{
					duplicatedScratch = true;
					break;
				}
			}
			if (true == duplicatedScratch)
			{
				continue;
			}

			Hitted.Add(hitCharacterScratch);
		}
	}

	const FHitData& hitSchemaScratch = HitDatas[RoarIndex];

	for (int32 damageIndexScratch = 0; damageIndexScratch < Hitted.Num(); ++damageIndexScratch)
	{
		ACCommonCharacter* const victimScratch = Hitted[damageIndexScratch].Get();
		if (false == IsValid(victimScratch))
		{
			continue;
		}

		if (true == UCYJJBlueprintLibrary::AreCharactersSameGroup(dragonScratch, victimScratch))
		{
			continue;
		}

		hitSchemaScratch.SendDamage(
			TWeakObjectPtr<ACCommonCharacter>(dragonScratch),
			TWeakObjectPtr<AActor>(this),
			TWeakObjectPtr<ACCommonCharacter>(victimScratch));
	}

	UWorld* const worldForShakeScratch = GetWorld();
	if (IsValid(worldForShakeScratch) && (false == worldForShakeScratch->IsNetMode(NM_DedicatedServer)))
	{
		if (IsValid(hitSchemaScratch.ShakeClass))
		{
			const float outerRadiusScratch = static_cast<float>(RoarRadius);
			// WorldContextObject 는 UObject* — UWorld* 직접 전달은 시그니처·리플렉션과 어긋날 수 있다.
			UGameplayStatics::PlayWorldCameraShake(
				this,
				hitSchemaScratch.ShakeClass,
				sphereOriginScratch,
				0.0f,
				outerRadiusScratch,
				1.0f,
				false);
		}
	}

	const FDoActionData* const montageRowForScheduleScratch =
		DoActionDatas.IsValidIndex(RoarIndex) ? &DoActionDatas[RoarIndex] : nullptr;
	const float scheduleEndScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
		montageRowForScheduleScratch,
		SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::Roar),
		5.0f,
		nullptr,
		1.0f);
	Skill_SchedulePhaseEnd(scheduleEndScratch);
}

void ACDragonWeapon::Skill_2_Implementation()
{
	ACDragon* const dragonScratch = Dragon_ResolveOwningDragon();
	if (false == IsValid(dragonScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_2(Fire) Dragon 없음 — %s"),
			*GetNameSafe(this)));
		Skill_SchedulePhaseEnd(1.0f);
		return;
	}

	const FDoActionData* const actionRowScratch =
		DoActionDatas.IsValidIndex(FireIndex) ? &DoActionDatas[FireIndex] : nullptr;
	if (actionRowScratch != nullptr)
	{
		if (IsValid(actionRowScratch->Montage))
		{
			dragonScratch->PlayAnimMontage(actionRowScratch->Montage, actionRowScratch->PlayRate);
		}
		if (IsValid(actionRowScratch->Sound))
		{
			actionRowScratch->PlaySoundWave(TWeakObjectPtr<ACCommonCharacter>(dragonScratch));
		}
		const TWeakObjectPtr<UWorld> worldWeak = dragonScratch->GetWorld();
		if ((true == worldWeak.IsValid()) && (IsValid(actionRowScratch->Effect)))
		{
			actionRowScratch->PlayEffect(worldWeak, dragonScratch->GetActorLocation(), dragonScratch->GetActorRotation());
		}
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_2(Fire) DoActionDatas 인덱스 없음 — FireIndex=%d, Num=%d — %s"),
			FireIndex,
			DoActionDatas.Num(),
			*GetNameSafe(this)));
	}

	if (FireSkillActorClass == nullptr)
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_2(Fire) FireSkillActorClass 미지정 — 몽타주 길이/대기만 적용 — %s"),
			*GetNameSafe(this)));
		const float scheduleScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
			actionRowScratch,
			SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::Fire),
			8.0f,
			nullptr,
			1.0f);
		Skill_SchedulePhaseEnd(scheduleScratch);
		return;
	}

	USkeletalMeshComponent* const meshScratch = dragonScratch->GetMesh();
	if (false == IsValid(meshScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_2(Fire) Dragon Mesh 없음 — %s"),
			*GetNameSafe(this)));
		const float scheduleScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
			actionRowScratch,
			SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::Fire),
			8.0f,
			nullptr,
			1.0f);
		Skill_SchedulePhaseEnd(scheduleScratch);
		return;
	}

	const FTransform socketTransformScratch =
		meshScratch->GetSocketTransform(DragonFireAttachSocketName, ERelativeTransformSpace::RTS_World);

	UWorld* const worldScratch = GetWorld();
	if (false == IsValid(worldScratch))
	{
		CLog::Log(FString::Printf(TEXT("[DragonWeapon] Skill_2(Fire) World 없음 — %s"), *GetNameSafe(this)));
		Skill_SchedulePhaseEnd(1.0f);
		return;
	}

	FActorSpawnParameters spawnParamsScratch;
	spawnParamsScratch.Owner = this;
	spawnParamsScratch.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* const spawnedFireScratch = worldScratch->SpawnActor<AActor>(FireSkillActorClass, socketTransformScratch, spawnParamsScratch);
	if (false == IsValid(spawnedFireScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_2(Fire) 스폰 실패 — 클래스=%s — %s"),
			*GetNameSafe(*FireSkillActorClass),
			*GetNameSafe(this)));
		const float scheduleScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
			actionRowScratch,
			SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::Fire),
			8.0f,
			nullptr,
			1.0f);
		Skill_SchedulePhaseEnd(scheduleScratch);
		return;
	}

	spawnedFireScratch->AttachToComponent(
		meshScratch,
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		DragonFireAttachSocketName);

	CurFire = spawnedFireScratch;

	static const FName fireStartName(TEXT("Fire"));
	if (false == Skill_TryProcessVoidFunction(CurFire.Get(), fireStartName))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_2(Fire) 대상 액터에 void Fire 이벤트 없음 — BP 확인 — %s"),
			*GetNameSafe(CurFire.Get())));
	}

	const float scheduleFireScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
		actionRowScratch,
		SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::Fire),
		8.0f,
		nullptr,
		1.0f);
	Skill_SchedulePhaseEnd(scheduleFireScratch);
}

void ACDragonWeapon::Skill_3_Implementation()
{
	ACDragon* const dragonScratch = Dragon_ResolveOwningDragon();
	if (false == IsValid(dragonScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_3(Meteor) Dragon 없음 — %s"),
			*GetNameSafe(this)));
		Skill_SchedulePhaseEnd(1.0f);
		return;
	}

	const FDoActionData* const actionRowScratch =
		DoActionDatas.IsValidIndex(MeteorIndex) ? &DoActionDatas[MeteorIndex] : nullptr;
	if (actionRowScratch != nullptr)
	{
		if (IsValid(actionRowScratch->Montage))
		{
			dragonScratch->PlayAnimMontage(actionRowScratch->Montage, actionRowScratch->PlayRate);
		}
		if (IsValid(actionRowScratch->Sound))
		{
			actionRowScratch->PlaySoundWave(TWeakObjectPtr<ACCommonCharacter>(dragonScratch));
		}
		const TWeakObjectPtr<UWorld> worldWeak = dragonScratch->GetWorld();
		if ((true == worldWeak.IsValid()) && (IsValid(actionRowScratch->Effect)))
		{
			actionRowScratch->PlayEffect(worldWeak, dragonScratch->GetActorLocation(), dragonScratch->GetActorRotation());
		}
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_3(Meteor) DoActionDatas 인덱스 없음 — MeteorIndex=%d, Num=%d — %s"),
			MeteorIndex,
			DoActionDatas.Num(),
			*GetNameSafe(this)));
	}

	if (MeteorSkillActorClass == nullptr)
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_3(Meteor) MeteorSkillActorClass 미지정 — %s"),
			*GetNameSafe(this)));
		const float scheduleScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
			actionRowScratch,
			SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::Meteor),
			8.0f,
			nullptr,
			1.0f);
		Skill_SchedulePhaseEnd(scheduleScratch);
		return;
	}

	UWorld* const worldScratch = GetWorld();
	if (false == IsValid(worldScratch))
	{
		CLog::Log(FString::Printf(TEXT("[DragonWeapon] Skill_3(Meteor) World 없음 — %s"), *GetNameSafe(this)));
		Skill_SchedulePhaseEnd(1.0f);
		return;
	}

	worldScratch->GetTimerManager().ClearTimer(DragonMeteorSpawnTimerHandle);
	Dragon_OnMeteorSpawnTick();

	const float periodSecondsScratch = FMath::Max(0.05f, MeteorSpawnPeriodSeconds);
	const FTimerDelegate meteorDelegateScratch =
		FTimerDelegate::CreateUObject(this, &ACDragonWeapon::Dragon_OnMeteorSpawnTick);
	worldScratch->GetTimerManager().SetTimer(DragonMeteorSpawnTimerHandle, meteorDelegateScratch, periodSecondsScratch, true);

	const float scheduleMeteorScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
		actionRowScratch,
		SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::Meteor),
		8.0f,
		nullptr,
		1.0f);
	Skill_SchedulePhaseEnd(scheduleMeteorScratch);
}

void ACDragonWeapon::Skill_4_Implementation()
{
	ACDragon* const dragonScratch = Dragon_ResolveOwningDragon();
	if (false == IsValid(dragonScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_4(FireFly) Dragon 없음 — %s"),
			*GetNameSafe(this)));
		Skill_SchedulePhaseEnd(1.0f);
		return;
	}

	USkeletalMeshComponent* const meshScratch = dragonScratch->GetMesh();
	USceneComponent* const neckScratch = dragonScratch->Neck.Get();

	CurSkillIndex = 3;

	if ((false == IsValid(neckScratch)) || (false == IsValid(meshScratch)))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_4(FireFly) Neck 또는 Mesh 없음 — FireAir 스폰만 시도 — %s"),
			*GetNameSafe(this)));
	}
	else if (false == DragonFireFlyAttachSocketName.IsNone())
	{
		const FAttachmentTransformRules neckRulesScratch(
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepRelative,
			true);
		neckScratch->AttachToComponent(meshScratch, neckRulesScratch, DragonFireFlyAttachSocketName);
		bDragonWeapon_FireFlyNeckWasReparentedForSkill4 = true;
	}

	if (FireFlySkillActorClass == nullptr)
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_4(FireFly) FireFlySkillActorClass 미지정 — 소켓·대기만 적용 — %s"),
			*GetNameSafe(this)));
		const float scheduleScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
			nullptr,
			SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::FireFly),
			15.0f,
			nullptr,
			1.0f);
		Skill_SchedulePhaseEnd(scheduleScratch);
		return;
	}

	if (false == IsValid(meshScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_4(FireFly) Mesh 없어 FireAir 부착 불가 — %s"),
			*GetNameSafe(this)));
		const float scheduleScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
			nullptr,
			SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::FireFly),
			15.0f,
			nullptr,
			1.0f);
		Skill_SchedulePhaseEnd(scheduleScratch);
		return;
	}

	UWorld* const worldScratch = GetWorld();
	if (false == IsValid(worldScratch))
	{
		CLog::Log(FString::Printf(TEXT("[DragonWeapon] Skill_4(FireFly) World 없음 — %s"), *GetNameSafe(this)));
		Skill_SchedulePhaseEnd(1.0f);
		return;
	}

	const FTransform spawnTransformScratch =
		meshScratch->GetSocketTransform(DragonFireFlyAttachSocketName, ERelativeTransformSpace::RTS_World);

	FActorSpawnParameters spawnParamsScratch;
	spawnParamsScratch.Owner = dragonScratch;
	spawnParamsScratch.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* const spawnedFireAirScratch =
		worldScratch->SpawnActor<AActor>(FireFlySkillActorClass, spawnTransformScratch, spawnParamsScratch);
	if (false == IsValid(spawnedFireAirScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_4(FireFly) 스폰 실패 — 클래스=%s — %s"),
			*GetNameSafe(*FireFlySkillActorClass),
			*GetNameSafe(this)));
		const float scheduleScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
			nullptr,
			SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::FireFly),
			15.0f,
			nullptr,
			1.0f);
		Skill_SchedulePhaseEnd(scheduleScratch);
		return;
	}

	CurFireAir = spawnedFireAirScratch;

	const FAttachmentTransformRules airSkillAttachRulesScratch(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepWorld,
		true);

	spawnedFireAirScratch->AttachToComponent(meshScratch, airSkillAttachRulesScratch, DragonFireFlyAttachSocketName);

	static const FName fireFlyingStartName(TEXT("Fire"));
	if (false == Skill_TryProcessVoidFunction(CurFireAir.Get(), fireFlyingStartName))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_4(FireFly) Skill_Dragon_FireAir 에 void Fire 없음 — BP 확인 — %s"),
			*GetNameSafe(CurFireAir.Get())));
	}

	const float scheduleFlyScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
		nullptr,
		SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::FireFly),
		15.0f,
		nullptr,
		1.0f);
	Skill_SchedulePhaseEnd(scheduleFlyScratch);

	Dragon_TryEnsureSplineActorSpawnedAtDragon(dragonScratch);
	Dragon_ClearFireFlyThunderLatentTimers();
	bDragonWeapon_FireFlyHasIssuedFlyingFunctionCallThisPhase = false;
	MovedLength = 0.0;
	Dragon_ApplyBossFlyJumpAndLaunchBack(dragonScratch);

	// 블프 `Wait` 상태로 두고, 레거시 `Delay`(1초) 뒤 스플라인 정렬은 타이머 한 번으로 처리 — 틱마다 `Delay` 가 재실행되는 블프 패턴을 피한다.
	FireFlyState = GDragonBossEFly_Wait;
	UWorld* const timerWorldScratch = GetWorld();
	if (IsValid(timerWorldScratch))
	{
		const FTimerDelegate waitDelegateScratch =
			FTimerDelegate::CreateUObject(this, &ACDragonWeapon::Dragon_OnFireFly_WaitBeforeSplinePhaseTimerExpired);
		timerWorldScratch->GetTimerManager().SetTimer(
			DragonFireFlyWaitBeforeEnterFlyTimerHandle,
			waitDelegateScratch,
			FMath::Max(0.01f, DragonBossFireFlyWaitBeforeSetSeconds),
			false);
	}
}

void ACDragonWeapon::Skill_5_Implementation()
{
	ACDragon* const dragonThunderScratch = Dragon_ResolveOwningDragon();
	if (false == IsValid(dragonThunderScratch))
	{
		CLog::Log(FString::Printf(TEXT("[DragonWeapon] Skill_5(Thunder) Dragon 없음 — %s"), *GetNameSafe(this)));
		FlyState = GDragonBossEFly_None;
		CurSkillIndex = 4;
		const float scheduleThunderScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
			nullptr,
			SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::Thunder),
			4.0f,
			nullptr,
			1.0f);
		Skill_SchedulePhaseEnd(scheduleThunderScratch);
		return;
	}

	Dragon_ClearFireFlyThunderLatentTimers();
	FlyState = GDragonBossEFly_None;
	CurSkillIndex = 4;

	const float scheduleThunderScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
		nullptr,
		SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::Thunder),
		4.0f,
		nullptr,
		1.0f);
	Skill_SchedulePhaseEnd(scheduleThunderScratch);

	Dragon_ApplyBossFlyJumpAndLaunchBack(dragonThunderScratch);
	UWorld* const thunderWorldScratch = GetWorld();
	if (IsValid(thunderWorldScratch))
	{
		const FTimerDelegate thunderLandDelegateScratch =
			FTimerDelegate::CreateUObject(this, &ACDragonWeapon::Dragon_OnFlyThunder_WaitThenLandTimerExpired);
		thunderWorldScratch->GetTimerManager().SetTimer(
			DragonFlyThunderWaitBeforeLandTimerHandle,
			thunderLandDelegateScratch,
			FMath::Max(0.01f, DragonBossThunderFlyPauseBeforeLandSeconds),
			false);
	}
}

void ACDragonWeapon::Skill_6_Implementation()
{
	ACDragon* const dragonScratch = Dragon_ResolveOwningDragon();
	if (false == IsValid(dragonScratch))
	{
		CLog::Log(FString::Printf(TEXT("[DragonWeapon] Skill_6(Fly 종료류) Dragon 없음 — %s"), *GetNameSafe(this)));
		Skill_SchedulePhaseEnd(1.0f);
		return;
	}

	CurSkillIndex = 5;

	const FDoActionData* const actionRowThunderScratch =
		DoActionDatas.IsValidIndex(ThunderIndex) ? &DoActionDatas[ThunderIndex] : nullptr;

	if (actionRowThunderScratch != nullptr)
	{
		if (IsValid(actionRowThunderScratch->Montage))
		{
			dragonScratch->PlayAnimMontage(actionRowThunderScratch->Montage, actionRowThunderScratch->PlayRate);
		}

		if (IsValid(actionRowThunderScratch->Sound))
		{
			actionRowThunderScratch->PlaySoundWave(TWeakObjectPtr<ACCommonCharacter>(dragonScratch));
		}

		const TWeakObjectPtr<UWorld> worldWeak = dragonScratch->GetWorld();
		if ((true == worldWeak.IsValid()) && IsValid(actionRowThunderScratch->Effect))
		{
			actionRowThunderScratch->PlayEffect(worldWeak, dragonScratch->GetActorLocation(), dragonScratch->GetActorRotation());
		}
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Skill_6 DoActionDatas 인덱스 없음 — ThunderIndex=%d Num=%d — 몽타주 없이 대기만 — %s"),
			ThunderIndex,
			DoActionDatas.Num(),
			*GetNameSafe(this)));
	}

	const float scheduleFlyScratch = Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
		actionRowThunderScratch,
		SkillType_ToSkillWaitTimeIndex(CEDragonBossSkillType::Fly),
		12.0f,
		nullptr,
		1.0f);
	Skill_SchedulePhaseEnd(scheduleFlyScratch);
}

void ACDragonWeapon::LandAttack_Implementation()
{
	if (false == HasAuthority())
	{
		return;
	}

	ACDragon* const dragonScratch = Dragon_ResolveOwningDragon();

	if (false == IsValid(dragonScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] LandAttack 불가 Dragon 없음 권위=%d — %s"),
			static_cast<int32>(HasAuthority() ? 1 : 0),
			*GetNameSafe(this)));
		return;
	}

	LandHitted.Empty();

	const FVector sphereCenter = dragonScratch->GetActorLocation();
	TArray<TEnumAsByte<EObjectTypeQuery>> objectQueriesScratch;
	objectQueriesScratch.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ignoreActorsScratch;
	ignoreActorsScratch.Add(dragonScratch);
	ignoreActorsScratch.Add(this);

	TArray<FHitResult> hitResultsScratch;
	const bool tracedScratch = UKismetSystemLibrary::SphereTraceMultiForObjects(
		this,
		sphereCenter,
		sphereCenter,
		static_cast<float>(LandAttackRadius),
		objectQueriesScratch,
		false,
		ignoreActorsScratch,
		EDrawDebugTrace::None,
		hitResultsScratch,
		true);

	if ((false == tracedScratch) || (hitResultsScratch.Num() <= 0))
	{
		return;
	}

	for (int32 hitIndexScratch = 0; hitIndexScratch < hitResultsScratch.Num(); ++hitIndexScratch)
	{
		FHitResult const& hitScratch = hitResultsScratch[hitIndexScratch];
		ACCommonCharacter* const hitCharacterScratch = Cast<ACCommonCharacter>(hitScratch.GetActor());
		if (false == IsValid(hitCharacterScratch))
		{
			continue;
		}

		bool duplicatedScratch = false;
		for (int32 landedIndexScratch = 0; landedIndexScratch < LandHitted.Num(); ++landedIndexScratch)
		{
			if (LandHitted[landedIndexScratch].Get() == hitCharacterScratch)
			{
				duplicatedScratch = true;
				break;
			}
		}
		if (true == duplicatedScratch)
		{
			continue;
		}

		LandHitted.Add(hitCharacterScratch);
	}

	for (int32 damageIndexScratch = 0; damageIndexScratch < LandHitted.Num(); ++damageIndexScratch)
	{
		ACCommonCharacter* const victimScratch = LandHitted[damageIndexScratch].Get();
		if (false == IsValid(victimScratch))
		{
			continue;
		}

		if (true == UCYJJBlueprintLibrary::AreCharactersSameGroup(dragonScratch, victimScratch))
		{
			continue;
		}

		FHitData hitDataScratch{};
		hitDataScratch.Damage = static_cast<float>(LandAttackDamage);
		hitDataScratch.AttackType = CEAttackType::Common;
		hitDataScratch.Montage = LandAttackMontage;
		hitDataScratch.PlayRate = 1.0f;
		hitDataScratch.bCanMove = false;
		hitDataScratch.Launch = 300.0f;
		hitDataScratch.CrowdControl = CECrowdControl::Down;
		hitDataScratch.HitStop = 0.0f;
		hitDataScratch.Sound = LandAttackSound;
		// 블프 FHitData.Effect 는 UFXSystemAsset 계열인데 LandAttackEffect 는 레거시 Cascade 파티클 — SendDamage 피격 VFX 슬롯은 비워 둔다(니아가라로 교체해 인스펙터 재지정 가능).
		hitDataScratch.Effect = nullptr;

		const FVector hitLocationScratch = victimScratch->GetActorLocation();
		hitDataScratch.EffectLocation = hitLocationScratch;

		hitDataScratch.SendDamage(
			TWeakObjectPtr<ACCommonCharacter>(dragonScratch),
			TWeakObjectPtr<AActor>(this),
			TWeakObjectPtr<ACCommonCharacter>(victimScratch));
	}

	LandHitted.Empty();
}

void ACDragonWeapon::Dragon_AbortSkillInFlight(ACDragon* const InDragonResolved)
{
	UWorld* const worldScratch = GetWorld();
	if (IsValid(worldScratch))
	{
		Dragon_ClearFireFlyThunderLatentTimers();
		worldScratch->GetTimerManager().ClearTimer(DragonSkillPhaseEndTimerHandle);
		worldScratch->GetTimerManager().ClearTimer(DragonMeteorSpawnTimerHandle);
	}

	Dragon_ResetFireFlyThunderRuntimeStateExceptSplineActor(true);

	Dragon_RuntimeTeardownSpawnedSkillActors();

	Dragon_RuntimeRestoreNeckAfterFireFlyIfNeeded(InDragonResolved);
}

void ACDragonWeapon::Dragon_ApplySkillPrelude(ACDragon* const InDragonResolved) const
{
	if (false == IsValid(InDragonResolved))
	{
		CLog::Log(FString::Printf(TEXT("[DragonWeapon] Prelude Dragon 없음 — %s"), *GetNameSafe(this)));
		return;
	}

	TWeakObjectPtr<UCMovementComponent> const movementWeak =
		YJJHelpers::GetComponent<UCMovementComponent>(InDragonResolved);
	if (false == movementWeak.IsValid())
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Prelude MovementComp 없음 — Dragon=%s"),
			*GetNameSafe(InDragonResolved)));
		return;
	}

	movementWeak->Stop();
	movementWeak->FixCamera();
}

void ACDragonWeapon::Dragon_ApplySkillPostlude(ACDragon* const InDragonResolved)
{
	Hitted.Empty();
	InAction = false;

	if (false == IsValid(InDragonResolved))
	{
		return;
	}

	TWeakObjectPtr<UCMovementComponent> const movementWeak =
		YJJHelpers::GetComponent<UCMovementComponent>(InDragonResolved);
	if (false == movementWeak.IsValid())
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Postlude MovementComp 없음 — Dragon=%s"),
			*GetNameSafe(InDragonResolved)));
		return;
	}

	movementWeak->Move();
	movementWeak->UnFixCamera();
}

ACDragon* ACDragonWeapon::Dragon_ResolveOwningDragon() const
{
	if (IsValid(Dragon.Get()))
	{
		return Dragon.Get();
	}

	return Cast<ACDragon>(CharacterOwner.Get());
}

int32 ACDragonWeapon::SkillType_ToSkillWaitTimeIndex(const CEDragonBossSkillType InSkillType) const
{
	return static_cast<int32>(InSkillType);
}

float ACDragonWeapon::Skill_PickScheduleSeconds_FromMontagePlayLengthOrFallback(
	const FDoActionData* const InMaybeRowOverride,
	const int32 FallbackSkillWaitTimeIndex,
	const float DefaultSecondsIfRowMissingMontage,
	UAnimMontage* const InExplicitMontage,
	const float InExplicitMontagePlayRate)
{
	TObjectPtr<UAnimMontage> pickedMontScratch = nullptr;
	float montageRateScratch = 1.0f;

	if (IsValid(InExplicitMontage))
	{
		pickedMontScratch = InExplicitMontage;
		montageRateScratch = InExplicitMontagePlayRate;
		if (true == FMath::IsNearlyZero(montageRateScratch))
		{
			montageRateScratch = 1.0f;
		}
	}
	else if (InMaybeRowOverride != nullptr)
	{
		if (IsValid(InMaybeRowOverride->Montage))
		{
			pickedMontScratch = InMaybeRowOverride->Montage;
			montageRateScratch = InMaybeRowOverride->PlayRate;
			if (true == FMath::IsNearlyZero(montageRateScratch))
			{
				montageRateScratch = 1.0f;
			}
		}
	}

	if (IsValid(pickedMontScratch))
	{
		const float lengthScratch = pickedMontScratch->GetPlayLength();
		return FMath::Max(0.05f, lengthScratch / montageRateScratch);
	}

	if (SkillWaitTime.IsValidIndex(FallbackSkillWaitTimeIndex))
	{
		return static_cast<float>(FMath::Max(0.05, SkillWaitTime[FallbackSkillWaitTimeIndex]));
	}

	return FMath::Max(0.05f, DefaultSecondsIfRowMissingMontage);
}

void ACDragonWeapon::Skill_SchedulePhaseEnd(const float InDelaySeconds)
{
	UWorld* const worldScratch = GetWorld();
	if (false == IsValid(worldScratch))
	{
		CLog::Log(FString::Printf(TEXT("[DragonWeapon] Skill_SchedulePhaseEnd World 없음 — %s"), *GetNameSafe(this)));
		return;
	}

	worldScratch->GetTimerManager().ClearTimer(DragonSkillPhaseEndTimerHandle);

	const float clampedDelayScratch = FMath::Max(0.05f, InDelaySeconds);
	const FTimerDelegate endDelegateScratch =
		FTimerDelegate::CreateUObject(this, &ACDragonWeapon::Dragon_OnSkillPhaseEndTimerExpired);
	worldScratch->GetTimerManager().SetTimer(DragonSkillPhaseEndTimerHandle, endDelegateScratch, clampedDelayScratch, false);
}

bool ACDragonWeapon::Skill_TryProcessVoidFunction(AActor* const InTargetActor, const FName& InFunctionName) const
{
	if (false == IsValid(InTargetActor))
	{
		return false;
	}

	UFunction* const functionScratch = InTargetActor->FindFunction(InFunctionName);
	if (nullptr == functionScratch)
	{
		return false;
	}

	InTargetActor->ProcessEvent(functionScratch, nullptr);
	return true;
}

void ACDragonWeapon::Skill_DestroyActorIfSpawnedSoft(AActor* const InActorCandidate)
{
	if (false == IsValid(InActorCandidate))
	{
		return;
	}

	InActorCandidate->Destroy();
}

void ACDragonWeapon::Dragon_RuntimeTeardownSpawnedSkillActors()
{
	if (IsValid(CurFire.Get()))
	{
		static const FName fireEndName(TEXT("End"));
		(void)Skill_TryProcessVoidFunction(CurFire.Get(), fireEndName);
		Skill_DestroyActorIfSpawnedSoft(CurFire.Get());
		CurFire = nullptr;
	}

	if (IsValid(CurFireAir.Get()))
	{
		static const FName fireAirEndName(TEXT("End"));
		(void)Skill_TryProcessVoidFunction(CurFireAir.Get(), fireAirEndName);
		Skill_DestroyActorIfSpawnedSoft(CurFireAir.Get());
		CurFireAir = nullptr;
	}

	if (IsValid(CurMeteor.Get()))
	{
		static const FName meteorEndName(TEXT("End"));
		(void)Skill_TryProcessVoidFunction(CurMeteor.Get(), meteorEndName);
		Skill_DestroyActorIfSpawnedSoft(CurMeteor.Get());
		CurMeteor = nullptr;
	}
}

void ACDragonWeapon::Dragon_RuntimeRestoreNeckAfterFireFlyIfNeeded(
	ACDragon* const InDragonResolvedOrNullUsesResolveFallback)
{
	if (false == bDragonWeapon_FireFlyNeckWasReparentedForSkill4)
	{
		return;
	}

	bDragonWeapon_FireFlyNeckWasReparentedForSkill4 = false;

	ACDragon* const dragonTargetScratch = IsValid(InDragonResolvedOrNullUsesResolveFallback)
		? InDragonResolvedOrNullUsesResolveFallback
		: Dragon_ResolveOwningDragon();

	if (IsValid(dragonTargetScratch))
	{
		dragonTargetScratch->ApplyNeckAttachToDragonMeshSocket(FName(TEXT("Fire_Ground")));
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] FireFly Neck 복귀 불가 Dragon 없음 — %s"),
			*GetNameSafe(this)));
	}
}

void ACDragonWeapon::Dragon_OnSkillPhaseEndTimerExpired()
{
	UWorld* const worldScratch = GetWorld();
	if (IsValid(worldScratch))
	{
		Dragon_ClearFireFlyThunderLatentTimers();
		worldScratch->GetTimerManager().ClearTimer(DragonMeteorSpawnTimerHandle);
	}

	Dragon_ResetFireFlyThunderRuntimeStateExceptSplineActor(true);

	Dragon_RuntimeTeardownSpawnedSkillActors();

	ACDragon* const dragonScratch = Dragon_ResolveOwningDragon();

	Dragon_RuntimeRestoreNeckAfterFireFlyIfNeeded(dragonScratch);

	if (IsValid(dragonScratch))
	{
		Dragon_ApplySkillPostlude(dragonScratch);
	}
	else
	{
		Hitted.Empty();
		InAction = false;
	}
}

void ACDragonWeapon::Dragon_OnMeteorSpawnTick()
{
	if (false == HasAuthority())
	{
		return;
	}

	if (MeteorSkillActorClass == nullptr)
	{
		return;
	}

	ACDragon* const dragonScratch = Dragon_ResolveOwningDragon();
	if (false == IsValid(dragonScratch))
	{
		return;
	}

	UWorld* const worldScratch = GetWorld();
	if (false == IsValid(worldScratch))
	{
		return;
	}

	USceneComponent* const neckScratch = dragonScratch->Neck.Get();
	FVector neckLocationScratch = dragonScratch->GetActorLocation();
	FRotator neckRotationScratch = dragonScratch->GetActorRotation();
	if (IsValid(neckScratch))
	{
		neckLocationScratch = neckScratch->GetComponentLocation();
		neckRotationScratch = neckScratch->GetComponentRotation();
	}

	const FVector forwardScratch = neckRotationScratch.Vector();
	const double halfRangeScratch = FMath::Max(0.0, MeteorAlongForwardRandomHalfRange);
	const float jitterForwardScratch = static_cast<float>(FMath::FRandRange(-halfRangeScratch, halfRangeScratch));
	const float baseForwardScratch = static_cast<float>(FMath::Max(800.0, halfRangeScratch));
	const FVector spawnLocationScratch = neckLocationScratch + forwardScratch * (baseForwardScratch + jitterForwardScratch);

	const FTransform spawnTransformScratch(neckRotationScratch, spawnLocationScratch);

	if (IsValid(CurMeteor.Get()))
	{
		Skill_DestroyActorIfSpawnedSoft(CurMeteor.Get());
		CurMeteor = nullptr;
	}

	FActorSpawnParameters spawnParamsScratch;
	spawnParamsScratch.Owner = this;
	spawnParamsScratch.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* const spawnedMeteorScratch = worldScratch->SpawnActor<AActor>(MeteorSkillActorClass, spawnTransformScratch, spawnParamsScratch);
	if (false == IsValid(spawnedMeteorScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Meteor 스폰 실패 — 클래스=%s — %s"),
			*GetNameSafe(*MeteorSkillActorClass),
			*GetNameSafe(this)));
		return;
	}

	CurMeteor = spawnedMeteorScratch;

	static const FName shootName(TEXT("Shoot"));
	if (false == Skill_TryProcessVoidFunction(CurMeteor.Get(), shootName))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Meteor 액터에 void Shoot 이벤트 없음 — BP 확인 — %s"),
			*GetNameSafe(CurMeteor.Get())));
	}
}

void ACDragonWeapon::Dragon_ClearFireFlyThunderLatentTimers()
{
	UWorld* const worldScratch = GetWorld();
	if (false == IsValid(worldScratch))
	{
		return;
	}

	worldScratch->GetTimerManager().ClearTimer(DragonFireFlyWaitBeforeEnterFlyTimerHandle);
	worldScratch->GetTimerManager().ClearTimer(DragonFireFlyDelayFireFlyingFunctionTimerHandle);
	worldScratch->GetTimerManager().ClearTimer(DragonFlyThunderWaitBeforeLandTimerHandle);
}

void ACDragonWeapon::Dragon_ResetFireFlyThunderRuntimeStateExceptSplineActor(bool const bIncludeMovedLengthReset)
{
	FireFlyState = GDragonBossEFly_None;
	FlyState = GDragonBossEFly_None;

	bDragonWeapon_FireFlyHasScheduledWaitStage = false;
	bDragonWeapon_FlyThunderLatentAwaitingLand = false;
	bDragonWeapon_FlyThunderPendingBurstJump = false;
	bDragonWeapon_FireFlyHasIssuedFlyingFunctionCallThisPhase = false;

	if (bIncludeMovedLengthReset)
	{
		MovedLength = 0.0;
	}
}

void ACDragonWeapon::Dragon_TryDiscoverPlayerTaggedActorIntoProperty()
{
	if (IsValid(Player.Get()))
	{
		return;
	}

	UWorld* const worldScratch = GetWorld();
	if (false == IsValid(worldScratch))
	{
		return;
	}

	TArray<AActor*> foundActorsScratch;
	UGameplayStatics::GetAllActorsWithTag(worldScratch, DragonPlayerDiscoveryActorTagName, foundActorsScratch);

	const int32 countScratch = foundActorsScratch.Num();
	for (int32 indexScratch = 0; indexScratch < countScratch; ++indexScratch)
	{
		AActor* tagActorScratch = foundActorsScratch[indexScratch];
		ACCommonCharacter* playerCandidateScratch = Cast<ACCommonCharacter>(tagActorScratch);
		if (IsValid(playerCandidateScratch))
		{
			Player = playerCandidateScratch;
			return;
		}
	}
}

void ACDragonWeapon::Dragon_TryEnsureSplineActorSpawnedAtDragon(ACDragon* const InDragon)
{
	if (IsValid(SplineActor.Get()))
	{
		return;
	}

	if (SplineClass == nullptr)
	{
		return;
	}

	if (false == IsValid(InDragon))
	{
		return;
	}

	UWorld* const worldScratch = GetWorld();
	if (false == IsValid(worldScratch))
	{
		return;
	}

	const FTransform spawnTransformScratch = InDragon->GetActorTransform();
	FActorSpawnParameters spawnParametersScratch;
	spawnParametersScratch.Owner = InDragon;
	spawnParametersScratch.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* const spawnedSplineScratch = worldScratch->SpawnActor<AActor>(SplineClass, spawnTransformScratch, spawnParametersScratch);
	if (IsValid(spawnedSplineScratch))
	{
		SplineActor = spawnedSplineScratch;
		return;
	}

	CLog::Log(FString::Printf(
		TEXT("[DragonWeapon] BP_Spline 스폰 실패 — SplineClass/월드 검사 — 무기=%s"),
		*GetNameSafe(this)));
}

USplineComponent* ACDragonWeapon::Dragon_FindFirstSplineComponentOnActor(AActor* const InActorCandidate)
{
	if (false == IsValid(InActorCandidate))
	{
		return nullptr;
	}

	return InActorCandidate->FindComponentByClass<USplineComponent>();
}

void ACDragonWeapon::Dragon_ApplyBossFlyJumpAndLaunchBack(ACDragon* const InDragonBoss)
{
	if (false == IsValid(InDragonBoss))
	{
		return;
	}

	UCStateComponent* const stateScratch = InDragonBoss->StateComp.Get();
	if (false == IsValid(stateScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] Boss FlyJump 생략 — StateComp 없음 — %s"),
			*GetNameSafe(InDragonBoss)));
		return;
	}

	InDragonBoss->Jump();
	stateScratch->SetFlying();

	UCMovementComponent* const movementScratch = InDragonBoss->GetYJJMovementComponent();
	if (IsValid(movementScratch))
	{
		// 레거시 `FlyJump`(FireAir)·Thunder 분기 블프 `InGravity` 기본값 1 과 동형.
		movementScratch->SetGravity(1.0f);
	}

	InDragonBoss->LaunchBack();
}

bool ACDragonWeapon::Dragon_StepFireFlyAlongSpline(ACDragon* const InDragonBoss, float const DeltaSeconds)
{
	if (false == IsValid(InDragonBoss))
	{
		return false;
	}

	USplineComponent* const splineScratch = Dragon_FindFirstSplineComponentOnActor(SplineActor.Get());
	if (nullptr == splineScratch)
	{
		CLog::Log(FString::Printf(TEXT("[DragonWeapon] FireFly 진행 불가 스플라인 미탑재 — %s"), *GetNameSafe(this)));
		Dragon_ArriveFinishFireFlyLandSequence(InDragonBoss);
		return true;
	}

	const float splineLengthScratch = splineScratch->GetSplineLength();
	if (splineLengthScratch <= KINDA_SMALL_NUMBER)
	{
		Dragon_ArriveFinishFireFlyLandSequence(InDragonBoss);
		return true;
	}

	UCMovementComponent* const mvScratch = InDragonBoss->GetYJJMovementComponent();
	if (false == IsValid(mvScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[DragonWeapon] FireFly 이동 스텝 중단 MovementComp 없음 — %s"),
			*GetNameSafe(InDragonBoss)));
		Dragon_ArriveFinishFireFlyLandSequence(InDragonBoss);
		return true;
	}

	const double deltaSecondsScratch = static_cast<double>(FMath::Max(0.f, DeltaSeconds));
	const float walkSpeedScratch = mvScratch->GetWalkSpeed();

	const double stepScratch =
		static_cast<double>(walkSpeedScratch) *
		static_cast<double>(DragonBossSplineAlongSpeedMultiplierVsWalk) * deltaSecondsScratch;

	const double nextMovedScratch = MovedLength + stepScratch;
	const double splineLenDoubleScratch = static_cast<double>(splineLengthScratch);

	if (nextMovedScratch >= splineLenDoubleScratch)
	{
		MovedLength = splineLenDoubleScratch;
		Dragon_ArriveFinishFireFlyLandSequence(InDragonBoss);
		return true;
	}

	MovedLength = nextMovedScratch;

	const float distanceAlongScratch = static_cast<float>(MovedLength);
	const FTransform worldTmScratch =
		splineScratch->GetTransformAtDistanceAlongSpline(distanceAlongScratch, ESplineCoordinateSpace::World, false);

	USceneComponent* const rootScratch = InDragonBoss->GetRootComponent();
	if (IsValid(rootScratch))
	{
		rootScratch->SetWorldLocationAndRotation(worldTmScratch.GetLocation(), worldTmScratch.Rotator());
		return false;
	}

	InDragonBoss->SetActorLocationAndRotation(worldTmScratch.GetLocation(), worldTmScratch.Rotator());
	return false;
}

void ACDragonWeapon::Dragon_ArriveFinishFireFlyLandSequence(ACDragon* const InDragonBoss)
{
	Dragon_ClearFireFlyThunderLatentTimers();

	FireFlyState = GDragonBossEFly_None;
	MovedLength = 0.0;
	bDragonWeapon_FireFlyHasIssuedFlyingFunctionCallThisPhase = false;

	static const FName skillFireAirEndName(TEXT("End"));
	(void)Skill_TryProcessVoidFunction(CurFireAir.Get(), skillFireAirEndName);

	static const FName finishSkillFourName(TEXT("Finish_Skill_4"));
	(void)Skill_TryProcessVoidFunction(this, finishSkillFourName);

	if (IsValid(InDragonBoss) && IsValid(InDragonBoss->FlyComp))
	{
		InDragonBoss->FlyComp->LandOn();
	}
}

void ACDragonWeapon::Dragon_ArriveFinishThunderFlyLandSequence(ACDragon* const InDragonBoss)
{
	UWorld* const worldScratch = GetWorld();
	if (IsValid(worldScratch))
	{
		worldScratch->GetTimerManager().ClearTimer(DragonFlyThunderWaitBeforeLandTimerHandle);
	}

	FlyState = GDragonBossEFly_None;

	static const FName finishSkillFiveName(TEXT("Finish_Skill_5"));
	(void)Skill_TryProcessVoidFunction(this, finishSkillFiveName);

	if (IsValid(InDragonBoss) && IsValid(InDragonBoss->FlyComp))
	{
		InDragonBoss->FlyComp->LandOn();
	}
}

void ACDragonWeapon::Dragon_TickFireFlyBossGraph(float const DeltaSeconds)
{
	if (false == HasAuthority())
	{
		return;
	}

	if (FireFlyState != GDragonBossEFly_Fly)
	{
		return;
	}

	ACDragon* const dragonScratch = Dragon_ResolveOwningDragon();
	if (false == IsValid(dragonScratch))
	{
		return;
	}

	(void)Dragon_StepFireFlyAlongSpline(dragonScratch, DeltaSeconds);
}

void ACDragonWeapon::Dragon_TickFlyThunderBossGraph(float const DeltaSeconds)
{
	(void)DeltaSeconds;
}

void ACDragonWeapon::Dragon_OnFireFly_WaitBeforeSplinePhaseTimerExpired()
{
	if (false == HasAuthority())
	{
		return;
	}

	ACDragon* const dragonScratch = Dragon_ResolveOwningDragon();
	if (false == IsValid(dragonScratch))
	{
		FireFlyState = GDragonBossEFly_None;
		CLog::Log(FString::Printf(TEXT("[DragonWeapon] FireFly 대기 후 Dragon 없음 — %s"), *GetNameSafe(this)));
		return;
	}

	USplineComponent* const splineScratch = Dragon_FindFirstSplineComponentOnActor(SplineActor.Get());
	if (nullptr == splineScratch)
	{
		FireFlyState = GDragonBossEFly_None;
		CLog::Log(FString::Printf(TEXT("[DragonWeapon] FireFly 스플라인 없음 시작 좌표 생략 — %s"), *GetNameSafe(this)));
		return;
	}

	const FTransform startTmScratch =
		splineScratch->GetTransformAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World, false);
	dragonScratch->SetActorLocationAndRotation(startTmScratch.GetLocation(), startTmScratch.Rotator());
	MovedLength = 0.0;

	FireFlyState = GDragonBossEFly_Fly;

	UWorld* const worldScratch = GetWorld();
	if (false == IsValid(worldScratch))
	{
		return;
	}

	const FTimerDelegate fireFlyingDelayedDelegateScratch =
		FTimerDelegate::CreateUObject(this, &ACDragonWeapon::Dragon_OnFireFly_DelayedFireFlyingFunctionTimerExpired);
	worldScratch->GetTimerManager().SetTimer(
		DragonFireFlyDelayFireFlyingFunctionTimerHandle,
		fireFlyingDelayedDelegateScratch,
		FMath::Max(0.01f, DragonBossFireFlyDelayBeforeFireFlyingFunctionSeconds),
		false);
}

void ACDragonWeapon::Dragon_OnFireFly_DelayedFireFlyingFunctionTimerExpired()
{
	if (false == HasAuthority())
	{
		return;
	}

	static const FName fireFlyingGraphName(TEXT("FireFlying"));

	if (true == bDragonWeapon_FireFlyHasIssuedFlyingFunctionCallThisPhase)
	{
		return;
	}

	bDragonWeapon_FireFlyHasIssuedFlyingFunctionCallThisPhase = true;

	if (false == Skill_TryProcessVoidFunction(this, fireFlyingGraphName))
	{
		CLog::Log(FString::Printf(TEXT("[DragonWeapon] void FireFlying 없음 — 무기 블프에 이벤트 확인 — %s"), *GetNameSafe(this)));
	}
}

void ACDragonWeapon::Dragon_OnFlyThunder_WaitThenLandTimerExpired()
{
	if (false == HasAuthority())
	{
		return;
	}

	ACDragon* const dragonScratch = Dragon_ResolveOwningDragon();
	Dragon_ArriveFinishThunderFlyLandSequence(dragonScratch);
}
