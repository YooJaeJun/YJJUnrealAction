#include "Characters/Animals/Dragon/Weapon/CDragonSkillFire.h"

#include "Characters/CCommonCharacter.h"
#include "Characters/Animals/Dragon/CDragon.h"
#include "Characters/Animals/Dragon/Weapon/CDragonWeapon.h"
#include "Commons/CYJJBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Utilities/CLog.h"

ACSkillDragonFire::ACSkillDragonFire()
{
	PrimaryActorTick.bCanEverTick = true;

	FireEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireEffect"));
	FireEffect->SetupAttachment(DefaultSceneRoot);
	FireEffect->PrimaryComponentTick.bStartWithTickEnabled = false;
	FireEffect->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FireCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("FireCollision"));
	FireCollision->SetupAttachment(DefaultSceneRoot);
	// 레거시 `End` 상태와 동일 — 실제 활성화는 `Fire()`에서 QueryOnly 로 연다.
	FireCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement =
		CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovement->InitialSpeed = 1200.0f;
	ProjectileMovement->bAutoActivate = false;
}

bool ACSkillDragonFire::DragonFire_ListContainsScratch(
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

bool ACSkillDragonFire::DragonFire_HasAuthorityForDamageScratch() const
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

void ACSkillDragonFire::DragonFire_CopyIgnoresScratch(
	const TArray<TObjectPtr<AActor>>& SourceActorsToIgnoreScratch,
	AActor* ExplicitCharacterScratch,
	TArray<AActor*>& OutIgnoresScratch)
{
	OutIgnoresScratch.Reset();

	const int32 sourceUpperScratch = SourceActorsToIgnoreScratch.Num();
	for (int32 ixScratch = 0; ixScratch < sourceUpperScratch; ++ixScratch)
	{
		if (AActor* entryScratch = SourceActorsToIgnoreScratch[ixScratch].Get())
		{
			bool duplicateScratch = false;
			const int32 builtUpperScratch = OutIgnoresScratch.Num();
			for (int32 iyScratch = 0; iyScratch < builtUpperScratch; ++iyScratch)
			{
				if (OutIgnoresScratch[iyScratch] == entryScratch)
				{
					duplicateScratch = true;
					break;
				}
			}
			if (false == duplicateScratch)
			{
				OutIgnoresScratch.Add(entryScratch);
			}
		}
	}

	if (IsValid(ExplicitCharacterScratch))
	{
		bool dupCharacterScratch = false;
		const int32 builtScratch = OutIgnoresScratch.Num();
		for (int32 izScratch = 0; izScratch < builtScratch; ++izScratch)
		{
			if (OutIgnoresScratch[izScratch] == ExplicitCharacterScratch)
			{
				dupCharacterScratch = true;
				break;
			}
		}

		if (false == dupCharacterScratch)
		{
			OutIgnoresScratch.Add(ExplicitCharacterScratch);
		}
	}
}

void ACSkillDragonFire::DragonFire_BindOverlapScratch()
{
	if (false == IsValid(FireCollision))
	{
		return;
	}

	FireCollision->OnComponentBeginOverlap.AddDynamic(this, &ACSkillDragonFire::DragonFire_OnFireOverlapScratch);
}

void ACSkillDragonFire::DragonFire_UnbindOverlapScratch()
{
	if (false == IsValid(FireCollision))
	{
		return;
	}

	FireCollision->OnComponentBeginOverlap.RemoveDynamic(this, &ACSkillDragonFire::DragonFire_OnFireOverlapScratch);
}

void ACSkillDragonFire::DragonFire_BindDeadDelegateScratch()
{
	ACCommonCharacter* const ccScratch = Character.Get();
	if (false == IsValid(ccScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[ACSkillDragonFire] Character 없음 — OnIsDead 바인드 생략(%s)."),
			*GetNameSafe(this)));
		return;
	}

	ccScratch->OnIsDead.AddDynamic(this, &ACSkillDragonFire::DragonFire_OnCharacterDeadDelegateScratch);
}

void ACSkillDragonFire::DragonFire_UnbindDeadDelegateScratch()
{
	ACCommonCharacter* const ccScratch = Character.Get();

	if (IsValid(ccScratch))
	{
		ccScratch->OnIsDead.RemoveDynamic(this, &ACSkillDragonFire::DragonFire_OnCharacterDeadDelegateScratch);
	}
}

void ACSkillDragonFire::DragonFire_OnCharacterDeadDelegateScratch()
{
	// 레거시 BP 가 Dragon 의 IsDead(깨졌던 디스패처) 에 End 를 연결했다 — 피격/주체 타깃이 죽으면 스킬 액터를 정리한다.
	Destroy();
}

void ACSkillDragonFire::DragonFire_UpdateAttachedBreathScratch()
{
	if (false == Attached)
	{
		return;
	}

	ACDragon* const dragonScratch = Dragon.Get();
	if ((false == IsValid(dragonScratch)) || (false == IsValid(dragonScratch->Neck)) || (false == IsValid(FireEffect)))
	{
		return;
	}

	FTransform sockTransformScratch =
		dragonScratch->Neck->GetSocketTransform(SocketName, RTS_World);
	FireEffect->SetWorldTransform(sockTransformScratch, false, nullptr, ETeleportType::None);
	DragonFire_OnAttachedBreathAfterTransformScratch();
}

void ACSkillDragonFire::DragonFire_OnFireOverlapScratch(
	UPrimitiveComponent* OverlappedComponentScratch,
	AActor* OtherActorScratch,
	UPrimitiveComponent* OtherCompScratch,
	int32 OtherBodyIndexScratch,
	bool bFromSweepScratch,
	const FHitResult& SweepResultScratch)
{
	(void)OverlappedComponentScratch;
	(void)OtherBodyIndexScratch;
	(void)bFromSweepScratch;
	(void)SweepResultScratch;

	if (false == DragonFire_HasAuthorityForDamageScratch())
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

	ACCommonCharacter* const hitScratch = Cast<ACCommonCharacter>(OtherActorScratch);
	if (false == IsValid(hitScratch))
	{
		return;
	}

	ACCommonCharacter* const attackerScratch = Character.Get();

	if ((false == IsValid(attackerScratch)) || true == UCYJJBlueprintLibrary::AreCharactersSameGroup(attackerScratch, hitScratch))
	{
		return;
	}

	if (true == DragonFire_ListContainsScratch(Hitted, hitScratch))
	{
		return;
	}

	ACDragonWeapon* const weaponScratch = DragonWeapon.Get();
	if (false == IsValid(weaponScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[ACSkillDragonFire] DragonWeapon 미해결 — 겹침 피격 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	const int32 fireIxScratch = weaponScratch->FireIndex;

	if (false == weaponScratch->HitDatas.IsValidIndex(fireIxScratch))
	{
		CLog::Log(FString::Printf(
			TEXT("[ACSkillDragonFire] HitDatas[%d] 범위 밖(Num=%d) — %s"),
			fireIxScratch,
			weaponScratch->HitDatas.Num(),
			*GetNameSafe(this)));
		return;
	}

	TArray<AActor*> ignoreScratchLocal;
	DragonFire_CopyIgnoresScratch(ActorsToIgnore, attackerScratch, ignoreScratchLocal);

	float sphereRadiusScratch = BetweenPrimitivesHitSphereRadius;
	if (IsValid(DragonWeapon))
	{
		sphereRadiusScratch = DragonWeapon->BetweenPrimitivesHitSphereRadius;
	}

	FVector tracedPointScratch = FVector::ZeroVector;
	const bool traceOkScratch =
		UCYJJBlueprintLibrary::TryGetHitPointBetweenPrimitives(
			FireCollision.Get(),
			OtherCompScratch,
			sphereRadiusScratch,
			ignoreScratchLocal,
			tracedPointScratch);

	if (false == traceOkScratch)
	{
		tracedPointScratch =
			IsValid(OtherCompScratch) ? OtherCompScratch->K2_GetComponentLocation() : hitScratch->GetActorLocation();
	}

	Hitted.Add(TObjectPtr<ACCommonCharacter>(hitScratch));

	const FHitData& rowScratch = weaponScratch->HitDatas[fireIxScratch];

	hitScratch->SetDamagedInfo(attackerScratch, this, rowScratch, tracedPointScratch);

	rowScratch.SendDamage(
		TWeakObjectPtr<ACCommonCharacter>(attackerScratch),
		TWeakObjectPtr<AActor>(this),
		TWeakObjectPtr<ACCommonCharacter>(hitScratch));

	if (IsValid(ShootSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShootSound, hitScratch->GetActorLocation());
	}
}

void ACSkillDragonFire::Fire()
{
	if (false == IsValid(FireCollision))
	{
		CLog::Log(FString::Printf(
			TEXT("[ACSkillDragonFire] FireCollision 없음 — Fire 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	FireCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FireCollision->SetVisibility(true, false);

	if (IsValid(LoopSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, LoopSound, GetActorLocation());
	}

	Attached = true;
}

void ACSkillDragonFire::DragonFire_End()
{
	if (false == IsValid(FireCollision))
	{
		CLog::Log(FString::Printf(
			TEXT("[ACSkillDragonFire] FireCollision 없음 — End 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	FireCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DragonFire_ApplyEndVfxScratch();

	Attached = false;
}

void ACSkillDragonFire::DragonFire_OnAfterBaselineFireScratch()
{
}

void ACSkillDragonFire::DragonFire_ApplyEndVfxScratch()
{
	if (false == IsValid(FireEffect))
	{
		return;
	}

	FireEffect->SetEmitterEnable(BreathEmitterToggleName, false);
}

void ACSkillDragonFire::DragonFire_OnAttachedBreathAfterTransformScratch()
{
}

void ACSkillDragonFire::BeginPlay()
{
	Super::BeginPlay();

	DragonFire_BindOverlapScratch();
	DragonFire_BindDeadDelegateScratch();
}

void ACSkillDragonFire::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DragonFire_UnbindDeadDelegateScratch();
	DragonFire_UnbindOverlapScratch();

	Super::EndPlay(EndPlayReason);
}

void ACSkillDragonFire::Tick(float DeltaSeconds)
{
	(void)DeltaSeconds;

	Super::Tick(DeltaSeconds);

	DragonFire_UpdateAttachedBreathScratch();
}
