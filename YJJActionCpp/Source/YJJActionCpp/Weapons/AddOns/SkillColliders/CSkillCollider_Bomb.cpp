#include "Weapons/AddOns/SkillColliders/CSkillCollider_Bomb.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/SphereComponent.h"
#include "Niagara/Public/NiagaraComponent.h"

ACSkillCollider_Bomb::ACSkillCollider_Bomb()
{
	YJJHelpers::CreateComponent<UCapsuleComponent>(this, &Capsule, "Capsule");
	YJJHelpers::CreateComponent<UParticleSystemComponent>(this, &Particle, "Particle", Capsule);
	YJJHelpers::CreateComponent<UParticleSystemComponent>(this, &Trail, "Trail", Particle);
	YJJHelpers::CreateActorComponent<UProjectileMovementComponent>(this, &Projectile, "Projectile");
	YJJHelpers::CreateComponent<USphereComponent>(this, &BombSphere, "BombSphere", Capsule);
	YJJHelpers::CreateComponent<UNiagaraComponent>(this, &BombParticle, "BombParticle", BombSphere);


	Capsule->SetCapsuleHalfHeight(44);
	Capsule->SetCapsuleRadius(44);

	InitialLifeSpan = 10;

	HitData.Launch = 0;
	HitData.Power = 5;
	YJJHelpers::GetAsset<UAnimMontage>(&HitData.Montage, "AnimMontage'/Game/Character/Player/Montages/Common/CHit_Stop_Montage.CHit_Stop_Montage'");

	Projectile->ProjectileGravityScale = 1.5f;
	Projectile->InitialSpeed = 1200;

	BombSphere->SetAutoActivate(false);
	BombSphere->SetSphereRadius(1000);
	BombSphere->SetCollisionProfileName("OverlapAll");
	BombSphere->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void ACSkillCollider_Bomb::BeginPlay()
{
	Super::BeginPlay();

	Forward = GetOwner()->GetActorForwardVector();

	const FVector spawnLocation = GetOwner()->GetActorLocation() + Forward * SpawnForwardLocationFactor;
	SetActorLocation(spawnLocation);

	Projectile->Velocity = Forward * Projectile->InitialSpeed;

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		this,
		&ACSkillCollider_Bomb::Bomb,
		BombRate,
		false);
}

void ACSkillCollider_Bomb::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void ACSkillCollider_Bomb::Bomb()
{
	BombSphere->Activate(true);
	BombSphere->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	
	if (IsValid(BombSphere) && BombSphere->IsActive())
	{
		BombSphere->OnComponentBeginOverlap.AddDynamic(this, &ACSkillCollider_Bomb::OnComponentBeginOverlap);
		BombSphere->OnComponentEndOverlap.AddDynamic(this, &ACSkillCollider_Bomb::OnComponentEndOverlap);
	}
}

void ACSkillCollider_Bomb::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

void ACSkillCollider_Bomb::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CheckTrue(GetOwner() == OtherActor);

	const TWeakObjectPtr<ACCommonCharacter> character = Cast<ACCommonCharacter>(OtherActor);
	if (character.IsValid())
		Hitted.Remove(character.Get());
}