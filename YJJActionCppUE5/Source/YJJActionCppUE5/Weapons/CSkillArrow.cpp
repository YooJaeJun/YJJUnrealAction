#include "Weapons/CSkillArrow.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialInterface.h"
#include "Utilities/YJJHelpers.h"
#include "Utilities/CLog.h"

namespace
{
	// 레거시 Skill_Arrow 컴포넌트 디폴트(블프 디테일 패널) — 에셋 패키지가 없으면 GetAssetDynamic 단계에서만 무시된다.
	static const TCHAR* const SkillArrowElvenBowMeshSoftPathScratch = TEXT(
		"StaticMesh'/Game/Assets/Character/Weapons/ElvenBow/SM_ElvenArrow.SM_ElvenArrow'");

	void SkillArrowTryApplyTrailCascadeDefaults(UParticleSystemComponent* InTrailParticle)
	{
		if (false == IsValid(InTrailParticle))
		{
			return;
		}

		// CSkillCollider_Bomb 과 동일한 Trail_Trace 콘텐츠 루트 — Red 가 아닌 기본 트레일.
		TObjectPtr<UParticleSystem> TrailTemplate = nullptr;
		// 패키지 미존재 시에는 조용히 nullptr — 블프/콘텐츠에서 트레일 지정 가능.
		YJJHelpers::GetAssetDynamic<UParticleSystem>(&TrailTemplate,
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
			YJJHelpers::GetAssetDynamic<UMaterialInterface>(&MatFace, FString(TrailMatPaths[Index]));
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
	PrimaryActorTick.bCanEverTick = false;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(DefaultSceneRoot);
	Sphere->SetSphereRadius(20.f);
	// UE5+: UShapeComponent 의 Area 오버라이드 필드가 protected — 여기선 내비만 막지 않도록 기본 채널로 둔다(지형 네비 상세 튜닝은 BP/별도 수정자 권장).
	Sphere->SetCanEverAffectNavigation(true);

	ArrowStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	ArrowStaticMesh->SetupAttachment(Sphere);
	ArrowStaticMesh->SetRelativeLocation(FVector(-45.f, 0.f, 0.f));
	ArrowStaticMesh->SetRelativeScale3D(FVector(1.f, 2.f, 2.f));
	ArrowStaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
	ArrowStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowStaticMesh->SetCollisionObjectType(ECC_WorldStatic);

	TObjectPtr<UStaticMesh> ArrowMeshAssetScratch = nullptr;
	YJJHelpers::GetAssetDynamic<UStaticMesh>(&ArrowMeshAssetScratch, FString(SkillArrowElvenBowMeshSoftPathScratch));
	if (IsValid(ArrowMeshAssetScratch))
	{
		ArrowStaticMesh->SetStaticMesh(ArrowMeshAssetScratch);
	}

	Trail = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Trail"));
	Trail->SetupAttachment(Sphere);
	Trail->SetRelativeScale3D(FVector(2.f, 2.f, 2.f));
	Trail->PrimaryComponentTick.bStartWithTickEnabled = false;
	Trail->SetAutoActivate(false);
	// 블프 bVisible=false — 활에 꽂힌 상태에서 트레일은 비표시이며 발사 후 Shoot 에서 연다.
	Trail->SetHiddenInGame(true);

	SkillArrowTryApplyTrailCascadeDefaults(Trail);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 8000.f;
	ProjectileMovement->MaxSpeed = 0.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->SetUpdatedComponent(Sphere.Get());
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
		ProjectileMovement->SetUpdatedComponent(Sphere.Get());
	}

	// 레거시 Bomb/FireBall 과 동일: 재활성화 시 속도 초기화·스위프 재시작 안정화.
	if (ProjectileMovement->IsActive())
	{
		ProjectileMovement->Deactivate();
	}

	ProjectileMovement->SetActive(false);

	const float SpeedScratch = ProjectileMovement->InitialSpeed;
	ProjectileMovement->Velocity = Normalized * SpeedScratch;

	ProjectileMovement->SetActive(true);

	if (IsValid(Trail))
	{
		Trail->SetHiddenInGame(false);
		Trail->Activate(true);
	}
}
