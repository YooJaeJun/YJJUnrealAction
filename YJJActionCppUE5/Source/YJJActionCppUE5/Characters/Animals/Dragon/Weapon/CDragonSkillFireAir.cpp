#include "Characters/Animals/Dragon/Weapon/CDragonSkillFireAir.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

ACSkillDragonFireAir::ACSkillDragonFireAir()
{
	SocketName = FName(TEXT("Fire_Air"));

	if (IsValid(FireEffect))
	{
		// 레거시 컴포넌트 기본값(CustomTimeDilation=2).
		FireEffect->CustomTimeDilation = 2.0f;
	}
}

void ACSkillDragonFireAir::DragonFire_OnAfterBaselineFireScratch()
{
	// 레거시 `End` 에서 고의로 가렸던 파티클을 다음 브레스에서 다시 켠다(`End` 후 `Fire`).
	if (IsValid(FireEffect))
	{
		FireEffect->SetVisibility(true, false);
	}

	const FVector atScratch = GetActorLocation();

	if (IsValid(ShootSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShootSound, atScratch);
	}
}

void ACSkillDragonFireAir::DragonFire_ApplyEndVfxScratch()
{
	if (false == IsValid(FireEffect))
	{
		return;
	}

	FireEffect->SetVisibility(false, false);
	FireEffect->SetEmitterEnable(BreathEmitterToggleName, false);
	FireEffect->SetEmitterEnable(BreathSmokeEmitterToggleName, false);
}

void ACSkillDragonFireAir::DragonFire_OnAttachedBreathAfterTransformScratch()
{
	if (false == IsValid(FireEffect))
	{
		return;
	}

	FireEffect->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
