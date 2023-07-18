#include "Weapons/AddOns/SkillColliders/CSkillCollider_Rotate.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Animation/AnimMontage.h"

ACSkillCollider_Rotate::ACSkillCollider_Rotate()
{
	PrimaryActorTick.bCanEverTick = true;

	YJJHelpers::CreateComponent<UCapsuleComponent>(this, &Capsule, "Capsule");
	YJJHelpers::CreateComponent<UParticleSystemComponent>(this, &Particle, "Particle", Capsule);

	Capsule->SetCapsuleHalfHeight(44);
	Capsule->SetCapsuleRadius(44);

	InitialLifeSpan = 7;

	HitData.Launch = 0;
	HitData.Power = 5;

	YJJHelpers::GetAsset<UAnimMontage>(&HitData.Montage, "AnimMontage'/Game/Character/Player/Montages/Common/CHit_Stop_Montage.CHit_Stop_Montage'");
}

void ACSkillCollider_Rotate::BeginPlay()
{
	Super::BeginPlay();

	Angle = UKismetMathLibrary::RandomFloatInRange(0, 360);

	if (bNegative && IsValid(Particle))
		Particle->SetRelativeRotation(FRotator(0, -180, 0));

	if (IsValid(Capsule))
	{
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &ACSkillCollider_Rotate::OnComponentBeginOverlap);
		Capsule->OnComponentEndOverlap.AddDynamic(this, &ACSkillCollider_Rotate::OnComponentEndOverlap);
	}

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle, 
		this, 
		&ACSkillCollider_Rotate::SendDamage, 
		DamageInteval, 
		true);
}

void ACSkillCollider_Rotate::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void ACSkillCollider_Rotate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector location = GetOwner()->GetActorLocation();

	Angle += (bNegative ? -Speed : +Speed) * DeltaTime;
	if (FMath::IsNearlyEqual(Angle, bNegative ? -360 : +360))
		Angle = 0;

	const FVector distance = FVector(Distance, 0, 0);
	const FVector value = distance.RotateAngleAxis(Angle, FVector::UpVector);

	location += value;

	SetActorLocation(location);
	SetActorRotation(FRotator(0, Angle, 0));
}

void ACSkillCollider_Rotate::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	CheckTrue(GetOwner() == OtherActor);

	const TWeakObjectPtr<ACCommonCharacter> character = Cast<ACCommonCharacter>(OtherActor);
	if (character.IsValid())
		Hitted.AddUnique(character.Get());
}

void ACSkillCollider_Rotate::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CheckTrue(GetOwner() == OtherActor);

	const TWeakObjectPtr<ACCommonCharacter> character = Cast<ACCommonCharacter>(OtherActor);
	if (character.IsValid())
		Hitted.Remove(character.Get());
}

void ACSkillCollider_Rotate::SendDamage()
{
	for (int32 i = Hitted.Num() - 1; i >= 0; i--)
		HitData.SendDamage(
			Cast<ACCommonCharacter>(GetOwner()), 
			this, 
			Hitted[i]);
}