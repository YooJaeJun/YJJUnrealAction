#include "Characters/Animals/Dragon/Weapon/CDragonSkillMeteor.h"

#include "Characters/CCommonCharacter.h"
#include "Characters/Animals/Dragon/CDragon.h"
#include "Characters/Animals/Dragon/Weapon/CDragonWeapon.h"
#include "Commons/CYJJBlueprintLibrary.h"
#include "Camera/CameraShakeBase.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Landscape.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Utilities/CLog.h"

namespace
{
	/** 레거시가 Dest 를 채우지 않은 블프·순수 C++ 테스트용: 전방으로 충분히 먼 점으로 보간되게 한다. */
	static constexpr float GDragonSkillMeteor_FallbackDestinationDistanceUU = 5000.f;
}

ACSkillDragonMeteor::ACSkillDragonMeteor()
{
	PrimaryActorTick.bCanEverTick = true;

	MeteorVisualPivot = CreateDefaultSubobject<USceneComponent>(TEXT("MeteorPivot"));
	MeteorVisualPivot->SetupAttachment(DefaultSceneRoot);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(MeteorVisualPivot);
	Sphere->SetRelativeLocation(FVector(0.f, 0.f, 25.f));
	Sphere->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	Sphere->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));
	Sphere->SetSphereRadius(55.f);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetGenerateOverlapEvents(true);
	Sphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	BombSphere = CreateDefaultSubobject<USphereComponent>(TEXT("BombSphere"));
	BombSphere->SetupAttachment(MeteorVisualPivot);
	BombSphere->SetSphereRadius(1000.f);
	BombSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BombSphere->SetGenerateOverlapEvents(false);
	BombSphere->SetCollisionProfileName(TEXT("NoCollision"));

	MeteorParticleMain =
		CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystem_GEN_VARIABLE"));
	MeteorParticleMain->SetupAttachment(Sphere);
	MeteorParticleMain->PrimaryComponentTick.bStartWithTickEnabled = false;
	MeteorParticleMain->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MeteorParticleTrail =
		CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Trail_GEN_VARIABLE"));
	MeteorParticleTrail->SetupAttachment(MeteorParticleMain);
	MeteorParticleTrail->PrimaryComponentTick.bStartWithTickEnabled = false;
	MeteorParticleTrail->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1200.f;
	ProjectileMovement->bAutoActivate = false;

	static ConstructorHelpers::FClassFinder<UCameraShakeBase> BombShakeFinder(TEXT("/Game/Magics/Bomb/CS_Bomb.CS_Bomb_C"));

	if (true == BombShakeFinder.Succeeded())
	{
		BombCameraShakeClass = BombShakeFinder.Class;
	}
}

bool ACSkillDragonMeteor::DragonMeteor_HasAuthorityForDamageScratch() const
{
	const AActor* authSourceScratch = Dragon.Get();
	if (false == IsValid(authSourceScratch))
	{
		authSourceScratch = DragonWeapon.Get();
	}
	if (false == IsValid(authSourceScratch))
	{
		authSourceScratch = GetOwner();
	}
	return IsValid(authSourceScratch) && authSourceScratch->HasAuthority();
}

bool ACSkillDragonMeteor::DragonMeteor_ListContainsScratch(
	const TArray<TObjectPtr<ACCommonCharacter>>& ListScratch,
	ACCommonCharacter* CandidateScratch)
{
	if (false == IsValid(CandidateScratch))
	{
		return false;
	}
	const int32 upperScratch = ListScratch.Num();
	for (int32 ixScratch = 0; ixScratch < upperScratch; ++ixScratch)
	{
		if (ListScratch[ixScratch].Get() == CandidateScratch)
		{
			return true;
		}
	}
	return false;
}

void ACSkillDragonMeteor::DragonMeteor_OnCharacterDeadDelegateScratch()
{
	// 복제 액터는 서버에서만 파괴해 클라 카피를 정렬한다(Bomb 동일 규약).
	if (true == HasAuthority())
	{
		Destroy();
	}
}

void ACSkillDragonMeteor::DragonMeteor_BindOverlapScratch()
{
	if (false == IsValid(Sphere))
	{
		return;
	}

	Sphere->OnComponentBeginOverlap.AddDynamic(this,
		&ACSkillDragonMeteor::DragonMeteor_OnProjectileSphereOverlapScratch);
}

void ACSkillDragonMeteor::DragonMeteor_UnbindOverlapScratch()
{
	if (false == IsValid(Sphere))
	{
		return;
	}

	Sphere->OnComponentBeginOverlap.RemoveDynamic(this,
		&ACSkillDragonMeteor::DragonMeteor_OnProjectileSphereOverlapScratch);
}

void ACSkillDragonMeteor::DragonMeteor_BindDeadDelegateScratch()
{
	ACCommonCharacter* const ccScratch = Character.Get();
	if (false == IsValid(ccScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[ACSkillDragonMeteor] Character 없음 — OnIsDead 바인드 생략(%s)."),
			*GetNameSafe(this)));
		return;
	}

	ccScratch->OnIsDead.AddDynamic(this, &ACSkillDragonMeteor::DragonMeteor_OnCharacterDeadDelegateScratch);
}

void ACSkillDragonMeteor::DragonMeteor_UnbindDeadDelegateScratch()
{
	ACCommonCharacter* const ccScratch = Character.Get();
	if (IsValid(ccScratch))
	{
		ccScratch->OnIsDead.RemoveDynamic(this, &ACSkillDragonMeteor::DragonMeteor_OnCharacterDeadDelegateScratch);
	}
}

void ACSkillDragonMeteor::DragonMeteor_TryBombFromOverlappedActorScratch(AActor* OtherActorScratch)
{
	if (false == IsValid(OtherActorScratch))
	{
		return;
	}

	if (false == DragonMeteor_HasAuthorityForDamageScratch())
	{
		// 레플 카피에서는 겹침이 들어와도 처리하지 않음(스팸·로그 과다 우려 — 틱 경로 주석 참고).
		return;
	}

	if (OtherActorScratch == GetOwner())
	{
		return;
	}

	bool bEligibleScratch = false;

	if (nullptr != Cast<ACCommonCharacter>(OtherActorScratch))
	{
		bEligibleScratch = true;
	}

	if ((false == bEligibleScratch) && (nullptr != Cast<ALandscape>(OtherActorScratch)))
	{
		bEligibleScratch = true;
	}

	if ((false == bEligibleScratch) && (nullptr != Cast<AStaticMeshActor>(OtherActorScratch)))
	{
		bEligibleScratch = true;
	}

	if (bEligibleScratch)
	{
		Bomb();
	}
}

void ACSkillDragonMeteor::DragonMeteor_OnProjectileSphereOverlapScratch(
	UPrimitiveComponent* OverlappedComponentScratch,
	AActor* OtherActorScratch,
	UPrimitiveComponent* OtherCompScratch,
	int32 OtherBodyIndexScratch,
	bool bFromSweepScratch,
	const FHitResult& SweepResultScratch)
{
	(void)OverlappedComponentScratch;
	(void)OtherCompScratch;
	(void)OtherBodyIndexScratch;
	(void)bFromSweepScratch;
	(void)SweepResultScratch;

	const AActor* const ownerScratch = GetOwner();

	if ((false == IsValid(ownerScratch)) || (OtherActorScratch == ownerScratch))
	{
		return;
	}

	DragonMeteor_TryBombFromOverlappedActorScratch(OtherActorScratch);
}

void ACSkillDragonMeteor::DragonMeteor_ApplyBombDamageAoEScratch(const FVector& ExplosionOriginScratch)
{
	UWorld* const worldScratch = GetWorld();
	if (false == IsValid(worldScratch))
	{
		CLog::Log(FString::Printf(TEXT("[ACSkillDragonMeteor] 폭발 피해: World 없음 — %s"), *GetNameSafe(this)));
		return;
	}

	ACDragonWeapon* const weaponScratch = DragonWeapon.Get();
	if (false == IsValid(weaponScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[ACSkillDragonMeteor] DragonWeapon 미해결 — AoE 피격 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	const int32 meteorIxScratch = weaponScratch->MeteorIndex;

	if (false == weaponScratch->HitDatas.IsValidIndex(meteorIxScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[ACSkillDragonMeteor] HitDatas[%d] 범위 밖(Num=%d) — %s"),
			meteorIxScratch,
			weaponScratch->HitDatas.Num(),
			*GetNameSafe(this)));
		return;
	}

	ACCommonCharacter* const attackerScratch = Character.Get();
	if (false == IsValid(attackerScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[ACSkillDragonMeteor] Character 없음 — AoE 피격 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypesScratch;

	// 레거시 BP SphereTrace Multi ObjectTypes 순서(Query3→1→2): 정적 월드·폰·다이나믹을 동일 채널로 환산.
	objectTypesScratch.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	objectTypesScratch.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	objectTypesScratch.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	TArray<AActor*> rawActorsToIgnoreScratch;
	rawActorsToIgnoreScratch.Reserve(ActorsToIgnore.Num() + 3);

	const int32 ignoreUpperScratch = ActorsToIgnore.Num();

	for (int32 ixIgScratch = 0; ixIgScratch < ignoreUpperScratch; ++ixIgScratch)
	{
		AActor* const ignActorScratch = ActorsToIgnore[ixIgScratch].Get();
		if (IsValid(ignActorScratch))
		{
			rawActorsToIgnoreScratch.Add(ignActorScratch);
		}
	}

	rawActorsToIgnoreScratch.AddUnique(this);

	if (AActor* const ownerScratchForIgnore = GetOwner())
	{
		rawActorsToIgnoreScratch.AddUnique(ownerScratchForIgnore);
	}

	TArray<FHitResult> traceHitsScratch;
	const float traceRadiusScratch = FMath::Max(64.f, BombDamageOverlapRadiusUU);

	const bool bAnyBlockingScratch =
		UKismetSystemLibrary::SphereTraceMultiForObjects(
			worldScratch,
			ExplosionOriginScratch,
			ExplosionOriginScratch,
			traceRadiusScratch,
			objectTypesScratch,
			false,
			rawActorsToIgnoreScratch,
			EDrawDebugTrace::None,
			traceHitsScratch,
			true);

	if ((false == bAnyBlockingScratch) || (traceHitsScratch.Num() == 0))
	{
		return;
	}

	const FHitData& rowScratch = weaponScratch->HitDatas[meteorIxScratch];
	const int32 hitsUpperScratch = traceHitsScratch.Num();

	for (int32 ixHitScratch = 0; ixHitScratch < hitsUpperScratch; ++ixHitScratch)
	{
		AActor* const hitActorScratch = traceHitsScratch[ixHitScratch].GetActor();

		ACCommonCharacter* const victimScratch = Cast<ACCommonCharacter>(hitActorScratch);
		if (false == IsValid(victimScratch))
		{
			continue;
		}

		if (true == UCYJJBlueprintLibrary::AreCharactersSameGroup(attackerScratch, victimScratch))
		{
			continue;
		}

		if (true == DragonMeteor_ListContainsScratch(Hitted, victimScratch))
		{
			continue;
		}

		Hitted.Add(TObjectPtr<ACCommonCharacter>(victimScratch));

		FVector launchDirScratch = ExplosionOriginScratch - victimScratch->GetActorLocation();
		launchDirScratch.Z = 0.f;

		if (launchDirScratch.SizeSquared() > KINDA_SMALL_NUMBER)
		{
			launchDirScratch.Normalize();
			victimScratch->LaunchCharacter(launchDirScratch * BombLaunchKnockbackSpeedUU, false, false);
		}

		const FVector impactPointScratch = traceHitsScratch[ixHitScratch].ImpactPoint;

		victimScratch->SetDamagedInfo(attackerScratch, this, rowScratch, impactPointScratch);

		rowScratch.SendDamage(
			TWeakObjectPtr<ACCommonCharacter>(attackerScratch),
			TWeakObjectPtr<AActor>(this),
			TWeakObjectPtr<ACCommonCharacter>(victimScratch));
	}
}

void ACSkillDragonMeteor::Bomb()
{
	if (true == bMeteorBombAlreadyTriggeredScratch)
	{
		return;
	}

	const FVector explosionOriginScratch = GetActorLocation();

	bMeteorBombAlreadyTriggeredScratch = true;

	bMotionEnabledScratch = false;

	if (IsValid(BombSphere.Get()))
	{
		BombSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	UNiagaraSystem* const bombFxScratchLocal = BombEffect.Get();
	UWorld* const worldScratch = GetWorld();

	const bool bRunCosmeticsScratch =
		IsValid(worldScratch) && worldScratch->GetNetMode() != NM_DedicatedServer;

	if ((true == bRunCosmeticsScratch) && IsValid(bombFxScratchLocal) && IsValid(worldScratch))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(worldScratch, bombFxScratchLocal, explosionOriginScratch);
	}

	if (IsValid(ProjectileMovement.Get()))
	{
		ProjectileMovement->ProjectileGravityScale = 0.f;
		if (ProjectileMovement->IsActive())
		{
			ProjectileMovement->Deactivate();
		}
	}

	if (IsValid(Sphere.Get()))
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (IsValid(MeteorParticleMain.Get()))
	{
		MeteorParticleMain->Deactivate();
	}

	if (IsValid(MeteorParticleTrail.Get()))
	{
		MeteorParticleTrail->Deactivate();
	}

	if ((true == bRunCosmeticsScratch) && IsValid(BombSound.Get()))
	{
		UGameplayStatics::PlaySoundAtLocation(this, BombSound, explosionOriginScratch);
	}

	if ((true == bRunCosmeticsScratch) &&
		(BombCameraShakeClass !=
		 nullptr) &&
		IsValid(worldScratch))
	{
		UGameplayStatics::PlayWorldCameraShake(
			this,
			BombCameraShakeClass,
			explosionOriginScratch,
			BombCameraShakeInnerRadiusUU,
			BombCameraShakeOuterRadiusUU,
			BombCameraShakeFalloff,
			false);
	}

	if (true == DragonMeteor_HasAuthorityForDamageScratch())
	{
		DragonMeteor_ApplyBombDamageAoEScratch(explosionOriginScratch);
	}

	if (true == HasAuthority())
	{
		Destroy();
	}
}

void ACSkillDragonMeteor::Shoot()
{
	bMotionEnabledScratch = true;

	CurScale = FVector::OneVector;

	if (IsValid(Sphere.Get()))
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Sphere->SetGenerateOverlapEvents(true);
		Sphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
		Sphere->SetWorldScale3D(CurScale);
	}

	if (IsValid(MeteorParticleMain.Get()))
	{
		MeteorParticleMain->Activate(true);
	}

	if (IsValid(MeteorParticleTrail.Get()))
	{
		MeteorParticleTrail->Activate(true);
	}

	if (IsValid(ShootSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShootSound, GetActorLocation());
	}

	if (IsValid(ProjectileMovement.Get()))
	{
		USceneComponent* const updatedScratch = Sphere.Get();
		ProjectileMovement->SetUpdatedComponent(updatedScratch);

		if (ProjectileMovement->IsActive())
		{
			ProjectileMovement->Deactivate();
		}

		// 블프 `SetActive(false)` 다음 `true` 순서 재현 — 실제 질량 이동은 틱에서 Dest 보간만 사용한다.
		ProjectileMovement->SetActive(false);
		ProjectileMovement->SetActive(true);
	}

	constexpr float nearlyZeroEpsilonScratch = 2.0f;
	if (Dest.IsNearlyZero(nearlyZeroEpsilonScratch))
	{
		const FVector forwardScratch =
			IsValid(Dragon.Get()) ?
				Dragon->GetActorForwardVector() :
			IsValid(Character.Get()) ?
				Character->GetActorForwardVector() :
				GetActorForwardVector();

		const FVector originScratch =
			IsValid(DefaultSceneRoot) ?
				DefaultSceneRoot->GetComponentLocation() :
				GetActorLocation();

		Dest =
			originScratch +
			(forwardScratch.GetSafeNormal() * GDragonSkillMeteor_FallbackDestinationDistanceUU);
	}
}

void ACSkillDragonMeteor::End()
{
	bMotionEnabledScratch = false;

	if (IsValid(ProjectileMovement.Get()) && ProjectileMovement->IsActive())
	{
		ProjectileMovement->Deactivate();
	}

	if (IsValid(MeteorParticleMain.Get()))
	{
		MeteorParticleMain->Deactivate();
	}

	if (IsValid(MeteorParticleTrail.Get()))
	{
		MeteorParticleTrail->Deactivate();
	}

	if (IsValid(Sphere.Get()))
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ACSkillDragonMeteor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (false == bMotionEnabledScratch || true == bMeteorBombAlreadyTriggeredScratch)
	{
		return;
	}

	USceneComponent* const rootScratch = GetRootComponent();
	if ((false == IsValid(rootScratch)) || (false == IsValid(DefaultSceneRoot)) || (false == IsValid(Sphere.Get())))
	{
		return;
	}

	if (IsValid(ProjectileMovement.Get()) && ProjectileMovement->IsActive())
	{
		ProjectileMovement->Deactivate();
	}

	if (IsValid(Sphere.Get()))
	{
		Sphere->SetWorldScale3D(CurScale);
	}

	CurScale = UKismetMathLibrary::Multiply_VectorFloat(
		CurScale,
		static_cast<double>(GrowScaleMultiplierPerTick));

	UWorld* const worldScratch = GetWorld();
	float const worldDtScratch = IsValid(worldScratch) ?
		worldScratch->GetDeltaSeconds() :
		DeltaSeconds;

	const FVector currentLocScratch = rootScratch->GetComponentLocation();

	const FVector nextLocScratch = UKismetMathLibrary::VInterpTo(
		currentLocScratch,
		Dest,
		worldDtScratch,
		MoveInterpSpeed);

	rootScratch->SetWorldLocation(nextLocScratch, false, nullptr, ETeleportType::TeleportPhysics);

	const float distRemainScratchSquared = FVector::DistSquared(nextLocScratch, Dest);
	static constexpr float kArriveEpsilonSqScratch = 25.f;

	if (distRemainScratchSquared <= kArriveEpsilonSqScratch)
	{
		Bomb();
	}
}

void ACSkillDragonMeteor::BeginPlay()
{
	Super::BeginPlay();

	DragonMeteor_BindOverlapScratch();
	DragonMeteor_BindDeadDelegateScratch();
}

void ACSkillDragonMeteor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	(void)EndPlayReason;

	if (ACDragonWeapon* const weaponScratch = DragonWeapon.Get())
	{
		if (weaponScratch->CurMeteor.Get() == this)
		{
			weaponScratch->CurMeteor = nullptr;
		}
	}

	DragonMeteor_UnbindDeadDelegateScratch();
	DragonMeteor_UnbindOverlapScratch();

	Super::EndPlay(EndPlayReason);
}
