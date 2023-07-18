#include "Weapons/AddOns/CArrow.h"
#include "Global.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/CapsuleComponent.h"

ACArrow::ACArrow()
{
	YJJHelpers::CreateComponent<UCapsuleComponent>(this, &Capsule, "Capsule");
	YJJHelpers::CreateActorComponent<UProjectileMovementComponent>(this, &Projectile, "Projectile");

	Projectile->ProjectileGravityScale = 1;
	Capsule->BodyInstance.bNotifyRigidBodyCollision = true;
	Capsule->SetCollisionProfileName("BlockAll");
}

void ACArrow::BeginPlay()
{
	Super::BeginPlay();

	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Capsule->OnComponentHit.AddDynamic(this, &ACArrow::OnComponentHit);

	Projectile->Deactivate();
}

void ACArrow::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (OnArrowEndPlay.IsBound())
		OnArrowEndPlay.Broadcast(this);
}

void ACArrow::Shoot(const FVector& InForward) const
{
	Projectile->Velocity = InForward * Projectile->InitialSpeed;
	Projectile->Activate();

	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ACArrow::OnComponentHit(
	UPrimitiveComponent* HitComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	FVector NormalImpulse, 
	const FHitResult& Hit)
{
	SetLifeSpan(LifeSpanAfterCollision);

	for (const TWeakObjectPtr<AActor> actor : Ignores)
		CheckTrue(actor == OtherActor);

	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	const TWeakObjectPtr<ACCommonCharacter> character = Cast<ACCommonCharacter>(OtherActor);
	CheckNull(character);

	const FName boneName = character->GetMesh()->FindClosestBone(Hit.ImpactPoint);
	AttachToComponent(
		OtherComp,
		FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget, 
			EAttachmentRule::KeepWorld, 
			EAttachmentRule::KeepWorld, 
			true),
		//FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
		boneName);

	if (OnArrowHit.IsBound() && character.IsValid())
		OnArrowHit.Broadcast(this, character.Get());
}