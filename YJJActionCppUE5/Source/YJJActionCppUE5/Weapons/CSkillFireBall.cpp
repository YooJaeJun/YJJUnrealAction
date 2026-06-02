#include "Weapons/CSkillFireBall.h"

#include "Characters/CCommonCharacter.h"
#include "Commons/CYJJBlueprintLibrary.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/HitResult.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Utilities/CLog.h"
#include "Utilities/YJJHelpers.h"
#include "Weapons/CSkillWeapon.h"

const TCHAR* const ACSkillFireBall::SkillFireBallParticleSoftPathScratch = TEXT(
	"ParticleAsset'/Game/Assets/Effects/AdvancedMagicFX12/particles/P_ky_shot_fire.P_ky_shot_fire'");



ACSkillFireBall::ACSkillFireBall()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(DefaultSceneRoot);
	Sphere->SetRelativeLocation(FVector(0.f, 0.f, 25.f));
	Sphere->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	Sphere->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));
	Sphere->SetSphereRadius(55.f);

	Sphere->SetCollisionProfileName(TEXT("OverlapAll"));
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetGenerateOverlapEvents(true);

	FireParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireParticle"));
	FireParticle->SetupAttachment(Sphere);
	FireParticle->PrimaryComponentTick.bStartWithTickEnabled = false;
	FireParticle->bAutoActivate = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = FMath::Max(1.f, FireBallInitialSpeedScratch);
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->SetUpdatedComponent(Sphere);
}

bool ACSkillFireBall::SkillFireBall_ShouldRunCosmeticsScratch() const
{
	UWorld const* WorldScratch = GetWorld();
	if (false == IsValid(WorldScratch))
	{
		return false;
	}
	const ENetMode NetModeScratch = WorldScratch->GetNetMode();

	return NM_DedicatedServer != NetModeScratch;
}

void ACSkillFireBall::SkillFireBall_BindParticleIfUnsetScratch()
{
	if (true == IsValid(FireBallParticleAsset.Get()))
	{
		return;
	}

	TObjectPtr<UParticleSystem> LoadedScratch = nullptr;
	YJJHelpers::GetAssetDynamic<UParticleSystem>(&LoadedScratch, FString(SkillFireBallParticleSoftPathScratch));
	FireBallParticleAsset = LoadedScratch.Get();
}

bool ACSkillFireBall::SkillFireBall_TargetLivingScratch(ACCommonCharacter* CandidateScratch) const
{
	if (false == IsValid(CandidateScratch))
	{
		return false;
	}

	UCCharacterStatComponent const* StatScratch = CandidateScratch->GetYJJCharacterStatComponent();

	if (false == IsValid(StatScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[SkillFireBall] StatComp 없음 — 생존 판정 불능, 피격 생략·소멸 — %s — %s"),
			*GetNameSafe(CandidateScratch),
			*GetNameSafe(this)));
		return false;
	}

	return false == StatScratch->IsDead();
}

void ACSkillFireBall::BeginPlay()
{
	SkillFireBall_BindParticleIfUnsetScratch();

	Super::BeginPlay();

	if (true == IsValid(FireParticle.Get()))
	{
		if (true == IsValid(FireBallParticleAsset.Get()))
		{
			FireParticle->SetTemplate(FireBallParticleAsset.Get());
		}
		FireParticle->Deactivate();
	}

	if (false == IsValid(Sphere.Get()))
	{
		CLog::Log(FString::Printf(TEXT("[SkillFireBall] Sphere 없음 — 오버랩 미바인드 — %s"), *GetNameSafe(this)));
		return;
	}

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ACSkillFireBall::SkillFireBall_OnSphereBeginOverlapScratch);

	if (true == IsValid(ProjectileMovement.Get()))
	{
		ProjectileMovement->SetUpdatedComponent(Sphere.Get());
		ProjectileMovement->InitialSpeed = FMath::Max(1.f, FireBallInitialSpeedScratch);
	}
}

void ACSkillFireBall::Shoot_Implementation(FVector const& InDirectionScratch)
{
	if (false == IsValid(Sphere.Get()) || false == IsValid(ProjectileMovement.Get()))
	{
		CLog::Log(FString::Printf(
			TEXT("[SkillFireBall] Shoot: Sphere 또는 ProjectileMovement 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	FVector ShootDirScratch = InDirectionScratch;

	if (false == ShootDirScratch.Normalize())
	{
		ACCommonCharacter* FallbackOwnerScratch = Character.Get();
		if (true == IsValid(FallbackOwnerScratch))
		{
			ShootDirScratch = FallbackOwnerScratch->GetActorForwardVector();
			if (false == ShootDirScratch.Normalize())
			{
				CLog::Log(FString::Printf(
					TEXT("[SkillFireBall] Shoot: 대체 진행 방향도 정규화 실패 — %s"),
					*GetNameSafe(this)));
				return;
			}
		}
		else
		{
			CLog::Log(FString::Printf(
				TEXT("[SkillFireBall] Shoot: InDirection 무효·Character 폴백 불가 — %s"),
				*GetNameSafe(this)));
			return;
		}
	}

	ProjectileMovement->InitialSpeed = FMath::Max(1.f, FireBallInitialSpeedScratch);
	const float SpeedScratch = ProjectileMovement->InitialSpeed;
	ProjectileMovement->Velocity = ShootDirScratch * SpeedScratch;

	if (true == ProjectileMovement->IsActive())
	{
		ProjectileMovement->Deactivate();
	}

	ProjectileMovement->SetUpdatedComponent(Sphere.Get());
	ProjectileMovement->SetActive(false);
	ProjectileMovement->SetActive(true);

	if (false == SkillFireBall_ShouldRunCosmeticsScratch())
	{
		return;
	}

	if (true == IsValid(FireParticle.Get()))
	{
		FireParticle->Activate(true);
	}
}

void ACSkillFireBall::SkillFireBall_OnSphereBeginOverlapScratch(
	UPrimitiveComponent* OverlappedComponentScratch,
	AActor* OtherActorScratch,
	UPrimitiveComponent* OtherCompScratch,
	int32 OtherBodyIndexScratch,
	bool bFromSweepScratch,
	FHitResult const& SweepResultScratch)
{
	(void)OverlappedComponentScratch;
	(void)OtherCompScratch;
	(void)OtherBodyIndexScratch;
	(void)bFromSweepScratch;
	(void)SweepResultScratch;

	if (false == HasAuthority())
	{
		return;
	}

	if (true == bSkillFireBallOverlapConsumedScratch)
	{
		return;
	}

	if (false == IsValid(OtherActorScratch))
	{
		return;
	}

	if (OtherActorScratch == this)
	{
		return;
	}

	ACCommonCharacter* VictimScratch = Cast<ACCommonCharacter>(OtherActorScratch);
	if (false == IsValid(VictimScratch))
	{
		// 레거시 캐스트 실패 핀과 동등 — 블프 `K2_DestroyActor`
		bSkillFireBallOverlapConsumedScratch = true;
		Destroy();

		return;
	}

	ACCommonCharacter* CasterScratch = Character.Get();

	if ((true == IsValid(CasterScratch)) && (VictimScratch == CasterScratch))
	{
		// 시전자 본체·스폰 직후 캡슐 겹침 때 스팸·자해 방지 — 틱은 없으므로 주석만(로컬 테스트에서만 간헐).
		return;
	}

	if ((true == IsValid(CasterScratch)) &&
		true == UCYJJBlueprintLibrary::AreCharactersSameGroup(CasterScratch, VictimScratch))
	{
		// 블프 `IsSameGroup` true 분기 — Damaged·Destroy 미연결, 투사체는 관통.
		return;
	}

	if ((true == IsValid(CasterScratch)) && (false == SkillFireBall_TargetLivingScratch(VictimScratch)))
	{
		bSkillFireBallOverlapConsumedScratch = true;

		Destroy();

		return;
	}

	const ACMagic* MagicScratch = Magic.Get();
	if ((false == IsValid(MagicScratch)) || (false == MagicScratch->HitCommonDatas.IsValidIndex(0)))
	{
		CLog::Log(FString::Printf(
			TEXT("[SkillFireBall] Magic 또는 HitCommonDatas[0] 없음 — Destroy — Magic=%s — %s"),
			*GetNameSafe(MagicScratch),
			*GetNameSafe(this)));

		bSkillFireBallOverlapConsumedScratch = true;

		Destroy();

		return;
	}

	if (false == IsValid(CasterScratch))
	{
		CLog::Log(FString::Printf(TEXT("[SkillFireBall] 피격 시점 Character(시전자) 없음 — Destroy — %s"), *GetNameSafe(this)));

		bSkillFireBallOverlapConsumedScratch = true;

		Destroy();

		return;
	}

	USphereComponent const* SphereCompScratch = Sphere.Get();
	FVector HitPointScratch = true == IsValid(SphereCompScratch) ? SphereCompScratch->GetComponentLocation()
																 : VictimScratch->GetActorLocation();

	FHitData const& RowScratch = MagicScratch->HitCommonDatas[0];

	VictimScratch->SetDamagedInfo(CasterScratch, this, RowScratch, HitPointScratch);

	RowScratch.SendDamage(
		TWeakObjectPtr<ACCommonCharacter>(CasterScratch),
		TWeakObjectPtr<AActor>(this),
		TWeakObjectPtr<ACCommonCharacter>(VictimScratch));

	bSkillFireBallOverlapConsumedScratch = true;

	Destroy();
}
