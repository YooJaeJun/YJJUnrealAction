#include "Weapons/AddOns/SkillColliders/CSkillCollider_Throw.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Animation/AnimMontage.h"

ACSkillCollider_Throw::ACSkillCollider_Throw()
{
	PrimaryActorTick.bCanEverTick = true;

	YJJHelpers::CreateComponent<UCapsuleComponent>(this, &Capsule, "Capsule");
	YJJHelpers::CreateComponent<UParticleSystemComponent>(this, &Particle, "Particle", Capsule);
	YJJHelpers::CreateActorComponent<UProjectileMovementComponent>(this, &Projectile, "Projectile");


	Capsule->SetCapsuleHalfHeight(44);
	Capsule->SetCapsuleRadius(44);

	InitialLifeSpan = 3;

	HitData.Launch = 0;
	HitData.Power = 5;
	YJJHelpers::GetAsset<UAnimMontage>(&HitData.Montage, "AnimMontage'/Game/Character/Player/Montages/Common/CHit_Stop_Montage.CHit_Stop_Montage'");

	Projectile->ProjectileGravityScale = 0;
	Projectile->InitialSpeed = 1200;
}

void ACSkillCollider_Throw::BeginPlay()
{
	Super::BeginPlay();

	Forward = GetOwner()->GetActorForwardVector();

	const FVector spawnLocation = GetOwner()->GetActorLocation() + Forward * SpawnForwardLocationFactor;
	SetActorLocation(spawnLocation);

	Projectile->Velocity = Forward * Projectile->InitialSpeed;


	if (IsValid(Capsule))
	{
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &ACSkillCollider_Throw::OnComponentBeginOverlap);
		Capsule->OnComponentEndOverlap.AddDynamic(this, &ACSkillCollider_Throw::OnComponentEndOverlap);
	}

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		this,
		&ACSkillCollider_Throw::SendDamage,
		DamageInteval,
		true);
}

void ACSkillCollider_Throw::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void ACSkillCollider_Throw::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACSkillCollider_Throw::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	CheckTrue(GetOwner() == OtherActor);

	const TWeakObjectPtr<ACCommonCharacter> character = Cast<ACCommonCharacter>(OtherActor);
	if (character.IsValid())
		Hitted.AddUnique(character.Get());
}

void ACSkillCollider_Throw::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CheckTrue(GetOwner() == OtherActor);

	const TWeakObjectPtr<ACCommonCharacter> character = Cast<ACCommonCharacter>(OtherActor);
	if (character.IsValid())
		Hitted.Remove(character.Get());
}

void ACSkillCollider_Throw::SendDamage()
{
	for (int32 i = Hitted.Num() - 1; i >= 0; i--)
		HitData.SendDamage(
			Cast<ACCommonCharacter>(GetOwner()),
			this,
			Hitted[i]);
}