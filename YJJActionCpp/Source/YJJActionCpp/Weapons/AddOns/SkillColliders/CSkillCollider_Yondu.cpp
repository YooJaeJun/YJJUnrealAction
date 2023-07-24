#include "Weapons/AddOns/SkillColliders/CSkillCollider_Yondu.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Components/SplineComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CCharacterInfoComponent.h"

ACSkillCollider_Yondu::ACSkillCollider_Yondu()
{
	YJJHelpers::CreateComponent<USceneComponent>(this, &ArrowRoot, "ArrowRoot");
	YJJHelpers::CreateComponent<UCapsuleComponent>(this, &Capsule, "Capsule", ArrowRoot);
	YJJHelpers::CreateComponent<UParticleSystemComponent>(this, &ParticleComp, "Particle", Capsule);
	YJJHelpers::CreateComponent<UNiagaraComponent>(this, &TrailComp, "Trail", Capsule);
	YJJHelpers::CreateComponent<USplineComponent>(this, &SplineComp, "Spline", ArrowRoot);

	YJJHelpers::GetAsset<UAnimMontage>(&HitData.Montage, "AnimMontage'/Game/Character/Player/Montages/Common/CHit_Stop_Montage.CHit_Stop_Montage'");
	YJJHelpers::GetAsset<UParticleSystem>(&ParticleAsset, "ParticleSystem'/Game/Assets/Effects/ArcherySystem/Assets/FX/Particles/Core/P_SingleTargetCore_Projectile.P_SingleTargetCore_Projectile'");
	YJJHelpers::GetAsset<UNiagaraSystem>(&TrailAsset, "NiagaraSystem'/Game/Effects/Yondu/NS_Yondu.NS_Yondu'");

	ParticleComp->Template = ParticleAsset;
	TrailComp->SetAsset(TrailAsset);


	Capsule->SetCapsuleHalfHeight(44);
	Capsule->SetCapsuleRadius(44);

	InitialLifeSpan = 4;

	HitData.Launch = 0;
	HitData.Power = 3;

	Capsule->OnComponentBeginOverlap.AddDynamic(this, &ACSkillCollider_Yondu::OnComponentBeginOverlap);
	Capsule->OnComponentEndOverlap.AddDynamic(this, &ACSkillCollider_Yondu::OnComponentEndOverlap);
}

void ACSkillCollider_Yondu::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACCommonCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	SetActorLocation(OwnerCharacter->GetActorLocation() + SpawnLocation);

	Targets.Empty();

	Shoot();
}

void ACSkillCollider_Yondu::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckFalse(bShot);

	if (MovedDistance < SplineComp->GetSplineLength())
	{
		const FTransform splineLoad = 
			SplineComp->GetTransformAtDistanceAlongSpline(MovedDistance, ESplineCoordinateSpace::World, false);

		ArrowRoot->SetWorldTransform(splineLoad);

		MovedDistance += GetWorld()->DeltaTimeSeconds * Speed;
	}
	else
		SetTarget();
}

void ACSkillCollider_Yondu::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ACSkillCollider_Yondu::Shoot()
{
	SetTarget();

	bShot = true;

	FTimerHandle timerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		timerHandle,
		[this]() {
			bShot = false;
		},
		EndTime,
			false);
}

void ACSkillCollider_Yondu::SetTarget()
{
	const FVector spawnLocation = OwnerCharacter->GetActorLocation() + SpawnLocation;
	SetActorLocation(spawnLocation);

	const TArray<AActor*> ignores{ OwnerCharacter.Get() };
	TArray<FHitResult> hitResults;
	const TArray<TEnumAsByte<EObjectTypeQuery>> query{ EObjectTypeQuery::ObjectTypeQuery3 };

	UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		GetActorLocation(),
		GetActorLocation(),
		TraceRadius,
		query,
		false,
		ignores,
		EDrawDebugTrace::None,
		hitResults,
		true);

	for (const FHitResult& hitResult : hitResults)
	{
		if (false == hitResult.bBlockingHit)
			continue;

		const TWeakObjectPtr<ACCommonCharacter> other =
			Cast<ACCommonCharacter>(hitResult.GetActor());

		const TWeakObjectPtr<UCStateComponent> state =
			YJJHelpers::GetComponent<UCStateComponent>(other.Get());

		const TWeakObjectPtr<UCCharacterInfoComponent> info =
			YJJHelpers::GetComponent<UCCharacterInfoComponent>(other.Get());

		if (true == info->IsSameGroup(OwnerCharacter.Get()))
			continue;

		if (true == Targets.Contains(other))
			continue;

		Targets.Add(other);

		SplineComp->AddSplinePoint(other->GetActorLocation(), ESplineCoordinateSpace::World, true);
	}
}

void ACSkillCollider_Yondu::OnComponentBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	CheckTrue(GetOwner() == OtherActor);

	const TWeakObjectPtr<ACCommonCharacter> character = Cast<ACCommonCharacter>(OtherActor);
	CheckNull(character);

	if (false == Hitted.Contains(character.Get()))
	{
		HitData.SendDamage(
			Cast<ACCommonCharacter>(GetOwner()),
			this,
			character.Get());
	}
}

void ACSkillCollider_Yondu::OnComponentEndOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex)
{
	CheckTrue(GetOwner() == OtherActor);

	const TWeakObjectPtr<ACCommonCharacter> character = Cast<ACCommonCharacter>(OtherActor);
	if (character.IsValid())
		Hitted.Remove(character.Get());
}