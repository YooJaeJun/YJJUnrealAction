#include "Weapons/AddOns/SkillColliders/CSkillCollider_Bomb.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/SphereComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShake.h"

ACSkillCollider_Bomb::ACSkillCollider_Bomb()
{
	YJJHelpers::CreateComponent<UCapsuleComponent>(this, &Capsule, "Capsule");
	YJJHelpers::CreateComponent<UParticleSystemComponent>(this, &Particle, "Particle", Capsule);
	YJJHelpers::CreateComponent<UParticleSystemComponent>(this, &Trail, "Trail", Particle);
	YJJHelpers::CreateActorComponent<UProjectileMovementComponent>(this, &Projectile, "Projectile");
	YJJHelpers::CreateComponent<USphereComponent>(this, &BombSphere, "BombSphere", Capsule);
	YJJHelpers::GetAsset<UFXSystemAsset>(&BombEffect, "NiagaraSystem'/Game/Assets/Effects/BigExplosions/Niagara/NS_Air_1.NS_Air_1'");
	YJJHelpers::GetAsset<USoundBase>(&BombSound, "SoundCue'/Game/Assets/Sounds/Explosion_Sounds_Pro_HD_Remake/Cues/Explosion_Massive_1_Cue.Explosion_Massive_1_Cue'");
	YJJHelpers::GetAsset<UAnimMontage>(&HitData.Montage, "AnimMontage'/Game/Character/Player/Montages/Common/CHit_Stop_Montage.CHit_Stop_Montage'");
	YJJHelpers::GetClass<UMatineeCameraShake>(&BombCameraShake, "Blueprint'/Game/Magics/Bomb/CS_Bomb.CS_Bomb_C'");

	Capsule->SetCapsuleHalfHeight(44);
	Capsule->SetCapsuleRadius(44);

	InitialLifeSpan = 4;

	HitData.Launch = 0;
	HitData.Power = 5;

	Projectile->ProjectileGravityScale = 0.5f;
	Projectile->InitialSpeed = 0;

	BombSphere->Activate(false);
	BombSphere->SetSphereRadius(1000);
	BombSphere->SetCollisionProfileName("OverlapAll");
	BombSphere->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);


	Capsule->OnComponentBeginOverlap.AddDynamic(this, &ACSkillCollider_Bomb::OnComponentBeginOverlap);

	BombSphere->OnComponentBeginOverlap.AddDynamic(this, &ACSkillCollider_Bomb::OnBombComponentBeginOverlap);
	BombSphere->OnComponentEndOverlap.AddDynamic(this, &ACSkillCollider_Bomb::OnBombComponentEndOverlap);
}

void ACSkillCollider_Bomb::BeginPlay()
{
	Super::BeginPlay();

	Direction = 
		GetWorld()->GetFirstPlayerController()->GetControlRotation().Vector() + 
		FVector(0, 0, 0.4f);

	const FVector spawnLocation = GetOwner()->GetActorLocation() + Direction * SpawnForwardLocationFactor;
	SetActorLocation(spawnLocation);

	Projectile->Velocity = FVector::ZeroVector;

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

void ACSkillCollider_Bomb::Throw() const
{
	Projectile->InitialSpeed = 1200;
	Projectile->Velocity = Direction * Projectile->InitialSpeed;
}

void ACSkillCollider_Bomb::Bomb() const
{
	if (IsValid(BombSphere))
	{
		BombSphere->Activate(true);
		BombSphere->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);

		if (IsValid(BombEffect))
		{
			YJJHelpers::PlayEffect(
				GetWorld(),
				Cast<UFXSystemAsset>(BombEffect),
				BombSphere->GetComponentTransform());

			UGameplayStatics::PlaySoundAtLocation(
				GetWorld(), 
				BombSound, 
				GetActorLocation(), 
				FRotator(0, 0, 0));

			UGameplayStatics::PlayWorldCameraShake(
				GetWorld(),
				BombCameraShake,
				GetActorLocation(),
				0,
				10000,
				1,
				false);
		}
	}
}

void ACSkillCollider_Bomb::OnComponentBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	Bomb();
}

void ACSkillCollider_Bomb::OnBombComponentBeginOverlap(
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

		// Causer가 발생시키는 Launch는 별도
		FVector launch = (GetActorLocation() - character->GetActorLocation()) * BombLaunch;
		launch.Z = 0;
		character->LaunchCharacter(launch, false, false);

		Hitted.Emplace(character.Get());
	}
}

void ACSkillCollider_Bomb::OnBombComponentEndOverlap(
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
