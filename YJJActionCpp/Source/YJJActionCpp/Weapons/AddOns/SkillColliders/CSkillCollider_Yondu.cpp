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
#include "Sound/SoundWave.h"

ACSkillCollider_Yondu::ACSkillCollider_Yondu()
{
	PrimaryActorTick.bCanEverTick = true;

	YJJHelpers::CreateComponent<USceneComponent>(this, &Root, "Root");
	YJJHelpers::CreateComponent<USceneComponent>(this, &ArrowRoot, "ArrowRoot", Root);
	YJJHelpers::CreateComponent<UCapsuleComponent>(this, &Capsule, "Capsule", ArrowRoot);
	YJJHelpers::CreateComponent<UParticleSystemComponent>(this, &ParticleComp, "Particle", Capsule);
	YJJHelpers::CreateComponent<UNiagaraComponent>(this, &TrailComp, "Trail", Capsule);
	YJJHelpers::CreateComponent<USplineComponent>(this, &SplineComp, "Spline", Root);

	YJJHelpers::GetAsset<UAnimMontage>(&HitData.Montage, "AnimMontage'/Game/Character/Player/Montages/Common/CHit_Stop_Montage.CHit_Stop_Montage'");
	YJJHelpers::GetAsset<UParticleSystem>(&ParticleAsset, "ParticleSystem'/Game/Assets/Effects/ArcherySystem/Assets/FX/Particles/Core/P_SingleTargetCore_Projectile.P_SingleTargetCore_Projectile'");
	YJJHelpers::GetAsset<UNiagaraSystem>(&TrailAsset, "NiagaraSystem'/Game/Effects/Yondu/NS_Yondu.NS_Yondu'");

	ParticleComp->Template = ParticleAsset;
	TrailComp->SetAsset(TrailAsset);


	Capsule->SetCapsuleHalfHeight(44);
	Capsule->SetCapsuleRadius(44);

	HitData.Launch = 0;
	HitData.Power = 10;
	YJJHelpers::GetAsset<UAnimMontage>(&HitData.Montage, "AnimMontage'/Game/Character/Player/Montages/Common/CHit_Stop_Montage.CHit_Stop_Montage'");
	YJJHelpers::GetAsset<USoundWave>(&HitData.Sound, "SoundWave'/Game/Assets/Sounds/Weapons/Yondu/Yondu_Hit.Yondu_Hit'");

	Capsule->OnComponentBeginOverlap.AddDynamic(this, &ACSkillCollider_Yondu::OnComponentBeginOverlap);
	Capsule->OnComponentEndOverlap.AddDynamic(this, &ACSkillCollider_Yondu::OnComponentEndOverlap);
}

void ACSkillCollider_Yondu::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ACCommonCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	Targets.Empty();

	ArrowRoot->SetWorldTransform(DefaultTransform);

	ProjectileState = CEProjectileState::Ready;
}

void ACSkillCollider_Yondu::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch (ProjectileState)
	{
	case CEProjectileState::Ready:
		ArrowRoot->SetWorldTransform(DefaultTransform);
		break;

	case CEProjectileState::Shooting:
		if (Tick_Move())
			ComeBack();
		break;

	case CEProjectileState::ComeBack:
		if (Tick_Move())
			Ready();
		break;

	default:
		break;
	}//switch(ProjectileState)
}

void ACSkillCollider_Yondu::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ACSkillCollider_Yondu::Shoot()
{
	SetActorTransform(DefaultTransform);

	SetTargets();

	ProjectileState = CEProjectileState::Shooting;
}

void ACSkillCollider_Yondu::ComeBack()
{
	ResetTargets();

	SplineComp->AddSplinePoint(
		ArrowRoot->GetComponentLocation(),
		ESplineCoordinateSpace::World);

	SplineComp->AddSplinePoint(
		DefaultTransform.GetLocation(),
		ESplineCoordinateSpace::World);

	ProjectileState = CEProjectileState::ComeBack;
}

void ACSkillCollider_Yondu::Ready()
{
	ResetTargets();

	ProjectileState = CEProjectileState::Ready;
}

bool ACSkillCollider_Yondu::Tick_Move()
{
	const FTransform splineLoad =
		SplineComp->GetTransformAtDistanceAlongSpline(
			MovedDistance,
			ESplineCoordinateSpace::World,
			false);

	ArrowRoot->SetWorldTransform(splineLoad);

	MovedDistance += GetWorld()->DeltaTimeSeconds * Speed;

	if (MovedDistance < SplineComp->GetSplineLength())
		return false;

	return true;
}

void ACSkillCollider_Yondu::SetTargets()
{
	ResetTargets();

	SplineComp->AddSplinePoint(
		GetActorLocation(),
		ESplineCoordinateSpace::World);

	const TArray<AActor*> ignores{ Owner.Get() };
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

		if (true == info->IsSameGroup(Owner.Get()))
			continue;

		if (true == Targets.Contains(other))
			continue;

		Targets.Add(other);

		SplineComp->AddSplinePoint(
			other->GetActorLocation(), 
			ESplineCoordinateSpace::World, 
			true);
	}
}

void ACSkillCollider_Yondu::ResetTargets()
{
	MovedDistance = 0;
	Targets.Empty();
	SplineComp->ClearSplinePoints();
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