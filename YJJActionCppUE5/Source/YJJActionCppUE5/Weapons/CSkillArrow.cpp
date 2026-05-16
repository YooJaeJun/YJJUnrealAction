#include "Weapons/CSkillArrow.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialInterface.h"
#include "Utilities/YJJHelpers.h"
#include "Utilities/CLog.h"

namespace
{
	void SkillArrowTryApplyTrailCascadeDefaults(UParticleSystemComponent* InTrailParticle)
	{
		if (false == IsValid(InTrailParticle))
		{
			return;
		}

		// CSkillCollider_Bomb 과 동일한 Trail_Trace 콘텐츠 루트 — Red 가 아닌 기본 트레일.
		TObjectPtr<UParticleSystem> TrailTemplate = nullptr;
		YJJHelpers::GetAsset<UParticleSystem>(
			&TrailTemplate,
			FString(TEXT("ParticleAsset'/Game/Assets/Effects/Trail_Trace/Blueprint_Splines/Effects/P_Trail_Trace.P_Trail_Trace'")));
		if (IsValid(TrailTemplate))
		{
			InTrailParticle->SetTemplate(TrailTemplate);
		}

		static const TCHAR* const TrailMatPaths[] = {
			TEXT("Material'/Game/Assets/Effects/Trail_Trace/Materials/M_Particle_Ribbon.M_Particle_Ribbon'"),
			TEXT("Material'/Game/Assets/Effects/Trail_Trace/Materials/m_flare_03.m_flare_03'"),
			TEXT("Material'/Game/Assets/Effects/Trail_Trace/Materials/m_flare_01.m_flare_01'"),
			TEXT("Material'/Game/Assets/Effects/Trail_Trace/Materials/M_radial_ramp.M_radial_ramp'"),
		};

		static const int32 TrailMatPathsCount =
			sizeof(TrailMatPaths) / sizeof(TrailMatPaths[0]);

		for (int32 Index = 0; Index < TrailMatPathsCount; ++Index)
		{
			TObjectPtr<UMaterialInterface> MatFace = nullptr;
			YJJHelpers::GetAsset<UMaterialInterface>(&MatFace, FString(TrailMatPaths[Index]));
			if (false == IsValid(MatFace))
			{
				continue;
			}

			InTrailParticle->SetMaterial(Index, MatFace.Get());
		}

		// 디테일 패널(Trail): ResetOnDetach 미체크, SecondsBeforeInactive 1.0
		InTrailParticle->bResetOnDetach = false;
		InTrailParticle->SecondsBeforeInactive = 1.0f;
	}
}

ACSkillArrow::ACSkillArrow()
{
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(DefaultSceneRoot);

	ArrowStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	ArrowStaticMesh->SetupAttachment(Sphere);

	Trail = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Trail"));
	Trail->SetupAttachment(Sphere);
	Trail->SetAutoActivate(true);

	SkillArrowTryApplyTrailCascadeDefaults(Trail);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 8000.f;
	ProjectileMovement->MaxSpeed = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->SetUpdatedComponent(Sphere);
}

void ACSkillArrow::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	OnArrowDestroied.Broadcast(this);
}

void ACSkillArrow::Shoot_Implementation(const FVector& InDirection)
{
	if (false == IsValid(ProjectileMovement))
	{
		CLog::Log(
			FString::Printf(TEXT("[SkillArrow] Shoot: ProjectileMovement 없음 — Actor=%s"), *GetNameSafe(this)));
		return;
	}

	const FVector Normalized = InDirection.GetSafeNormal();
	if (Normalized.IsNearlyZero())
	{
		CLog::Log(FString::Printf(
			TEXT("[SkillArrow] Shoot: 방향 벡터가 0 에 가깝습니다 — Actor=%s"), *GetNameSafe(this)));
		return;
	}

	if (IsValid(Sphere))
	{
		ProjectileMovement->SetUpdatedComponent(Sphere);
	}

	ProjectileMovement->Velocity = Normalized * ProjectileMovement->InitialSpeed;
	ProjectileMovement->SetActive(true);
}
