#include "Weapons/CSkillBomb.h"

#include "Characters/CCommonCharacter.h"
#include "Commons/CYJJBlueprintLibrary.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialInterface.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Utilities/CLog.h"
#include "Utilities/YJJHelpers.h"

namespace
{
 constexpr float BombPivotScaleUniformScratch = 0.7f;
 constexpr float BombSphereRadiusUUScratch = 55.f;

 static const TCHAR* const FireBallProjectileCascadePathScratch =
 TEXT("ParticleAsset'/Game/Assets/Effects/EasySurvivalRPG/Effects/PS_FireBall_Projectile.PS_FireBall_Projectile'");
 static const TCHAR* const BombSoundCuePathScratch = TEXT(
 "SoundCue'/Game/Assets/Sounds/Explosion_Sounds_Pro_HD_Remake/Cues/Explosion_Massive_1_Cue.Explosion_Massive_1_Cue'");
 static const TCHAR* const BombNiagaraSystemPathScratch =
 TEXT("NiagaraSystem'/Game/Assets/Effects/BigExplosions/Niagara/NS_Air_1.NS_Air_1'");
 static const TCHAR* const BombCameraShakeClassPathScratch =
 TEXT("/Game/Magics/Bomb/CS_Bomb.CS_Bomb_C");

	void BombTryBindSoftAssetsIfUnsetScratch(ACSkillBomb* SelfScratch)
	{
	 	if (nullptr == SelfScratch)
	 	{
	 		return;
	 	}

	 	if (false == IsValid(SelfScratch->BombSound.Get()))
	 	{
	 		TObjectPtr<USoundBase> loadedSoundScratch = nullptr;
	 		YJJHelpers::GetAssetDynamic<USoundBase>(&loadedSoundScratch, FString(BombSoundCuePathScratch));
	 		SelfScratch->BombSound = loadedSoundScratch.Get();
	 	}

	 	if (false == IsValid(SelfScratch->BombEffect.Get()))
	 	{
	 		TObjectPtr<UNiagaraSystem> loadedFxScratch = nullptr;
	 		YJJHelpers::GetAssetDynamic<UNiagaraSystem>(&loadedFxScratch, FString(BombNiagaraSystemPathScratch));
	 		SelfScratch->BombEffect = loadedFxScratch.Get();
	 	}
	}

 void BombTryActivateFireballCascadeTemplateScratch(UParticleSystemComponent* FireScratch)
 {
 	if (false == IsValid(FireScratch))
 	{
 		return;
 	}

 	TObjectPtr<UParticleSystem> fireTplScratch = nullptr;
 	YJJHelpers::GetAssetDynamic<UParticleSystem>(&fireTplScratch, FString(FireBallProjectileCascadePathScratch));
 	if (true == IsValid(fireTplScratch.Get()))
 	{
 		FireScratch->SetTemplate(fireTplScratch.Get());
 	}

 	FireScratch->PrimaryComponentTick.bStartWithTickEnabled = false;
 	FireScratch->bAutoActivate = false;
 }
} // namespace

void ACSkillBomb::SkillBombTryApplyProjectileTrailCascadeDefaults(UParticleSystemComponent* TrailParticleScratch)
{
	if (false == IsValid(TrailParticleScratch))
	{
		return;
	}

	TObjectPtr<UParticleSystem> trailTemplateScratch = nullptr;
	YJJHelpers::GetAssetDynamic<UParticleSystem>(
		&trailTemplateScratch,
		FString(
			TEXT("ParticleAsset'/Game/Assets/Effects/Trail_Trace/Blueprint_Splines/Effects/P_Trail_Trace.P_Trail_Trace'")));
	if (true == IsValid(trailTemplateScratch.Get()))
	{
		TrailParticleScratch->SetTemplate(trailTemplateScratch.Get());
	}

	static const TCHAR* const TrailMatPathsScratch[] = {
		TEXT("Material'/Game/Assets/Effects/Trail_Trace/Materials/M_Particle_Ribbon.M_Particle_Ribbon'"),
		TEXT("Material'/Game/Assets/Effects/Trail_Trace/Materials/m_flare_03.m_flare_03'"),
		TEXT("Material'/Game/Assets/Effects/Trail_Trace/Materials/m_flare_01.m_flare_01'"),
		TEXT("Material'/Game/Assets/Effects/Trail_Trace/Materials/M_radial_ramp.M_radial_ramp'"),
	};
	static const int32 TrailMatCountScratch = static_cast<int32>(sizeof(TrailMatPathsScratch) / sizeof(TCHAR*));

	for (int32 matIxScratch = 0; matIxScratch < TrailMatCountScratch; ++matIxScratch)
	{
		TObjectPtr<UMaterialInterface> matFaceScratch = nullptr;
		YJJHelpers::GetAssetDynamic<UMaterialInterface>(&matFaceScratch, FString(TrailMatPathsScratch[matIxScratch]));
		if (false == IsValid(matFaceScratch.Get()))
		{
			continue;
		}
		TrailParticleScratch->SetMaterial(matIxScratch, matFaceScratch.Get());
	}

	TrailParticleScratch->bResetOnDetach = false;
	TrailParticleScratch->SecondsBeforeInactive = 1.f;
	TrailParticleScratch->PrimaryComponentTick.bStartWithTickEnabled = false;
	TrailParticleScratch->bAutoActivate = false;
}

ACSkillBomb::ACSkillBomb()
{
	PrimaryActorTick.bCanEverTick = false;

	BombPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BombPivot"));
	BombPivot->SetupAttachment(DefaultSceneRoot);
	BombPivot->SetRelativeLocation(FVector(0.f, 0.f, 25.f));
	BombPivot->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	BombPivot->SetRelativeScale3D(FVector(BombPivotScaleUniformScratch));

	BombSphere = CreateDefaultSubobject<USphereComponent>(TEXT("BombSphere"));
	BombSphere->SetupAttachment(BombPivot);
	BombSphere->SetSphereRadius(BombSphereRadiusUUScratch);
	BombSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	BombSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BombSphere->SetGenerateOverlapEvents(false);

	ProjectileFireballParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ProjectileFireballParticle"));
	ProjectileFireballParticle->SetupAttachment(BombSphere);

	ProjectileTrailParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ProjectileTrailParticle"));
	ProjectileTrailParticle->SetupAttachment(BombSphere);

	BombTryBindSoftAssetsIfUnsetScratch(this);
	BombTryActivateFireballCascadeTemplateScratch(ProjectileFireballParticle.Get());
	SkillBombTryApplyProjectileTrailCascadeDefaults(ProjectileTrailParticle.Get());

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1200.f;
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->ProjectileGravityScale = 1.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->SetUpdatedComponent(BombSphere.Get());

	ConstructorHelpers::FClassFinder<UCameraShakeBase> ShakeFinder(BombCameraShakeClassPathScratch);
	if ((true == ShakeFinder.Succeeded()) && (nullptr != ShakeFinder.Class))
	{
		BombCameraShakeClass = ShakeFinder.Class;
	}
}

void ACSkillBomb::BeginPlay()
{
	BombTryBindSoftAssetsIfUnsetScratch(this);

	BombTryActivateFireballCascadeTemplateScratch(ProjectileFireballParticle.Get());

	Super::BeginPlay();

	if (IsValid(ProjectileMovement.Get()))
	{
		if (ProjectileMovement->IsActive())
		{
			ProjectileMovement->Deactivate();
		}
	}

	if (IsValid(BombSphere.Get()))
	{
		BombSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void ACSkillBomb::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	(void)EndPlayReason;

	SkillBomb_ClearFuseTimersScratch();

	Super::EndPlay(EndPlayReason);
}

bool ACSkillBomb::Bomb_ShouldRunCosmeticsScratch() const
{
	UWorld const* WorldScratch = GetWorld();

	if (false == IsValid(WorldScratch))
	{
		return false;
	}

	const ENetMode NetModeScratch = WorldScratch->GetNetMode();

	return NM_DedicatedServer != NetModeScratch;
}

bool ACSkillBomb::Bomb_HasAuthorityForDamageScratch() const
{
	const ENetRole RoleScratch = GetLocalRole();

	return ROLE_Authority == RoleScratch;
}

void ACSkillBomb::SkillBomb_RebuildActorsToIgnoreScratch()
{
	ActorsToIgnore.Empty();

	ActorsToIgnore.AddUnique(this);

	ACCommonCharacter* CharacterScratch = Character.Get();
	if (true == IsValid(CharacterScratch))
	{
		ActorsToIgnore.AddUnique(CharacterScratch);

		if (true == IsValid(CharacterScratch->CurInteractingActor))
		{
			ActorsToIgnore.AddUnique(CharacterScratch->CurInteractingActor.Get());
		}
	}
}

bool ACSkillBomb::SkillBomb_HittedAlreadyContainsScratch(
	const TArray<TObjectPtr<ACCommonCharacter>>& HittedScratch, ACCommonCharacter* CandidateScratch)
{
	if (nullptr == CandidateScratch)
	{
		return false;
	}

	const int32 UpperScratch = HittedScratch.Num();
	for (int32 ixScratch = 0; ixScratch < UpperScratch; ++ixScratch)
	{
		if (CandidateScratch == HittedScratch[ixScratch].Get())
		{
			return true;
		}
	}

	return false;
}

void ACSkillBomb::SkillBomb_ClearFuseTimersScratch()
{
	UWorld* WorldScratch = GetWorld();
	if (false == IsValid(WorldScratch))
	{
		return;
	}

	FTimerManager& timersScratch = WorldScratch->GetTimerManager();
	timersScratch.ClearTimer(FuseToBombTimerHandle);
	timersScratch.ClearTimer(DestroyAfterBombTimerHandle);
}

void ACSkillBomb::SkillBomb_TimerCallback_FuseBomb()
{
	Bomb();
}

void ACSkillBomb::SkillBomb_TimerCallback_DestroyAfterBomb()
{
	Destroy();
}

void ACSkillBomb::Bomb_ApplyDamageAoEScratch(UWorld* WorldScratch, const FVector& ExplosionOriginScratch)
{
	(void)WorldScratch;

	Hitted.Reset();

	const ACMagicSkillContext* MagicScratch = Magic.Get();
	ACCommonCharacter* MutableAttackerScratch = Character.Get();
	if ((false == IsValid(MagicScratch)) || (false == IsValid(MutableAttackerScratch)))
	{
		CLog::Log(FString::Printf(TEXT("[SkillBomb] Magic 또는 Character 미설정 — AoE 피격 생략 — %s"), *GetNameSafe(this)));
		return;
	}

	if (false == MagicScratch->HitCommonDatas.IsValidIndex(0))
	{
		CLog::Log(FString::Printf(
			TEXT("[SkillBomb] Magic.HitCommonDatas[0] 없음 — 피격 생략 — Magic=%s — %s"),
			*GetNameSafe(MagicScratch),
			*GetNameSafe(this)));
		return;
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectParamsScratch;

	ObjectParamsScratch.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	ObjectParamsScratch.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	ObjectParamsScratch.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	TArray<AActor*> RawIgnoreScratch;

	const int32 IgnoreUpperScratch = ActorsToIgnore.Num();
	RawIgnoreScratch.Reserve(IgnoreUpperScratch + 4);
	for (int32 igIxScratch = 0; igIxScratch < IgnoreUpperScratch; ++igIxScratch)
	{
		AActor* ScratchActor = ActorsToIgnore[igIxScratch].Get();
		if (true == IsValid(ScratchActor))
		{
			RawIgnoreScratch.Add(ScratchActor);
		}
	}

	RawIgnoreScratch.AddUnique(this);

	if (AActor* OwnerScratch = GetOwner())
	{
		RawIgnoreScratch.AddUnique(OwnerScratch);
	}

	TArray<FHitResult> TraceHitsScratch;

	const float TraceRadiusScratch = FMath::Max(64.f, BombSphereTraceRadiusUU);

	const bool bHadAnyHitScratch = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		ExplosionOriginScratch,
		ExplosionOriginScratch,
		TraceRadiusScratch,
		ObjectParamsScratch,
		false,
		RawIgnoreScratch,
		EDrawDebugTrace::None,
		TraceHitsScratch,
		true);

	if ((false == bHadAnyHitScratch) || (TraceHitsScratch.Num() <= 0))
	{
		return;
	}

	const FHitData& RowScratch = MagicScratch->HitCommonDatas[0];

	const int32 HitsUpperScratch = TraceHitsScratch.Num();
	for (int32 hitIxScratch = 0; hitIxScratch < HitsUpperScratch; ++hitIxScratch)
	{
		const FHitResult& HitScratch = TraceHitsScratch[hitIxScratch];
		if (false == HitScratch.bBlockingHit)
		{
			continue;
		}

		ACCommonCharacter* VictimScratch = Cast<ACCommonCharacter>(HitScratch.GetActor());

		if (false == IsValid(VictimScratch))
		{
			continue;
		}

		const bool SameGroupScratch = UCYJJBlueprintLibrary::AreCharactersSameGroup(
			MutableAttackerScratch, VictimScratch);
		if (true == SameGroupScratch)
		{
			continue;
		}

		if (true == SkillBomb_HittedAlreadyContainsScratch(Hitted, VictimScratch))
		{
			continue;
		}

		Hitted.Add(TObjectPtr<ACCommonCharacter>(VictimScratch));

		// 레거시 `BombLocation - VictimLocation` 에 Z=0 과 정규화를 적용 — 드래곤 메테오와 동형.
		FVector LaunchDirScratch = ExplosionOriginScratch - VictimScratch->GetActorLocation();
		LaunchDirScratch.Z = 0.f;

		if (LaunchDirScratch.SizeSquared() > KINDA_SMALL_NUMBER)
		{
			LaunchDirScratch.Normalize();
			VictimScratch->LaunchCharacter(LaunchDirScratch * BombLaunchKnockbackSpeedUU, false, false);
		}

		const FVector ImpactPointScratch = HitScratch.ImpactPoint;

		VictimScratch->SetDamagedInfo(MutableAttackerScratch, this, RowScratch, ImpactPointScratch);

		RowScratch.SendDamage(
			TWeakObjectPtr<ACCommonCharacter>(MutableAttackerScratch),
			TWeakObjectPtr<AActor>(this),
			TWeakObjectPtr<ACCommonCharacter>(VictimScratch));
	}
}

void ACSkillBomb::Bomb()
{
	UWorld* WorldScratch = GetWorld();
	if (false == IsValid(WorldScratch))
	{
		CLog::Log(FString::Printf(TEXT("[SkillBomb] Bomb 호출 시 World 없음 — %s"), *GetNameSafe(this)));
		return;
	}

	if (true == bBombExecutedScratch)
	{
		return;
	}

	bBombExecutedScratch = true;

	SkillBomb_ClearFuseTimersScratch();

	const FVector ExplosionOriginScratch = GetActorLocation();

	if (true == Bomb_HasAuthorityForDamageScratch())
	{
		Hitted.Reset();
	}

	if (IsValid(BombSphere.Get()))
	{
		BombSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		BombSphere->SetNotifyRigidBodyCollision(true);
	}

	if (true == Bomb_ShouldRunCosmeticsScratch())
	{
		if (true == IsValid(BombEffect.Get()))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(WorldScratch, BombEffect.Get(), ExplosionOriginScratch);
		}
		else
		{
			CLog::Log(FString::Printf(
				TEXT("[SkillBomb] BombEffect 미설정 — 나이아라 폭발 생략 — %s"), *GetNameSafe(this)));
		}
	}

	if (IsValid(ProjectileMovement.Get()))
	{
		ProjectileMovement->ProjectileGravityScale = 0.f;
		if (ProjectileMovement->IsActive())
		{
			ProjectileMovement->Deactivate();
		}
	}

	if (true == Bomb_ShouldRunCosmeticsScratch())
	{
		if (true == IsValid(BombSound.Get()))
		{
			UGameplayStatics::PlaySoundAtLocation(this, BombSound, ExplosionOriginScratch);
		}
	}

	if ((true == Bomb_ShouldRunCosmeticsScratch()) &&
		(nullptr != BombCameraShakeClass.Get()) &&
		IsValid(WorldScratch))
	{
		UGameplayStatics::PlayWorldCameraShake(
			this,
			BombCameraShakeClass.Get(),
			ExplosionOriginScratch,
			BombCameraShakeInnerRadiusUU,
			BombCameraShakeOuterRadiusUU,
			BombCameraShakeFalloff,
			false);
	}

	if (true == Bomb_HasAuthorityForDamageScratch())
	{
		Bomb_ApplyDamageAoEScratch(WorldScratch, ExplosionOriginScratch);
	}

	if (true == Bomb_ShouldRunCosmeticsScratch())
	{
		if (true == IsValid(ProjectileFireballParticle.Get()))
		{
			ProjectileFireballParticle->DeactivateSystem();
			ProjectileFireballParticle->Deactivate();
		}

		if (true == IsValid(ProjectileTrailParticle.Get()))
		{
			ProjectileTrailParticle->DeactivateSystem();
			ProjectileTrailParticle->Deactivate();
		}
	}

	if (IsValid(BombSphere.Get()))
	{
		BombSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BombSphere->SetGenerateOverlapEvents(false);
		BombSphere->SetNotifyRigidBodyCollision(false);
	}

	FTimerManager& timersScratch = WorldScratch->GetTimerManager();
	const FTimerDelegate DestroyDelegateScratch =
		FTimerDelegate::CreateUObject(this, &ACSkillBomb::SkillBomb_TimerCallback_DestroyAfterBomb);
	timersScratch.SetTimer(
		DestroyAfterBombTimerHandle,
		DestroyDelegateScratch,
		BombPostExplosionDestroyDelaySeconds,
		false);
}

void ACSkillBomb::Shoot_Implementation(FVector const& InDirection)
{
	UWorld* WorldScratch = GetWorld();
	if (false == IsValid(WorldScratch))
	{
		CLog::Log(FString::Printf(TEXT("[SkillBomb] Shoot: World 없음 — %s"), *GetNameSafe(this)));
		return;
	}

	SkillBomb_RebuildActorsToIgnoreScratch();

	if (true == bBombExecutedScratch)
	{
		return;
	}

	SkillBomb_ClearFuseTimersScratch();

	FVector ShootDirScratch = InDirection;

	if (false == ShootDirScratch.Normalize())
	{
		ACCommonCharacter* CharacterFacingScratch = Character.Get();
		if (true == IsValid(CharacterFacingScratch))
		{
			ShootDirScratch = CharacterFacingScratch->GetActorForwardVector();
			ShootDirScratch.Normalize();
		}
		else
		{
			CLog::Log(FString::Printf(
				TEXT("[SkillBomb] Shoot: 방향 정규화 실패 후 Character 로 대체 불가 — %s"),
				*GetNameSafe(this)));
			return;
		}
	}

	if ((false == IsValid(ProjectileMovement.Get())) || (false == IsValid(BombSphere.Get())))
	{
		CLog::Log(FString::Printf(TEXT("[SkillBomb] Shoot: ProjectileMovement 또는 BombSphere 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	const float SpeedScratch = ProjectileMovement->InitialSpeed;
	ProjectileMovement->Velocity = ShootDirScratch * SpeedScratch;

	if (ProjectileMovement->IsActive())
	{
		ProjectileMovement->Deactivate();
	}
	ProjectileMovement->SetUpdatedComponent(BombSphere.Get());

	ProjectileMovement->SetActive(false);
	ProjectileMovement->SetActive(true);

	if (true == Bomb_ShouldRunCosmeticsScratch())
	{
		if (true == IsValid(ProjectileFireballParticle.Get()))
		{
			ProjectileFireballParticle->Activate(true);
		}
		if (true == IsValid(ProjectileTrailParticle.Get()))
		{
			ProjectileTrailParticle->Activate(true);
		}
	}

	const FTimerDelegate FuseDelegateScratch =
		FTimerDelegate::CreateUObject(this, &ACSkillBomb::SkillBomb_TimerCallback_FuseBomb);
	WorldScratch->GetTimerManager().SetTimer(FuseToBombTimerHandle, FuseDelegateScratch, BombFuseDelaySeconds, false);
}
