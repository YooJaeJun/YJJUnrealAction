#include "Weapons/CWeaponStructures.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/CCharacterStatComponent.h"

void FEquipmentData::PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner) const
{
	for (const auto& sound : Sounds)
		CheckNull(sound);

	const TWeakObjectPtr<UWorld> world = InOwner->GetWorld();
	CheckNull(world);

	const FVector location = InOwner->GetActorLocation();

	for (const auto& sound : Sounds)
		UGameplayStatics::SpawnSoundAtLocation(world.Get(), sound, location);
}

void FActData::Act(const TWeakObjectPtr<ACCommonCharacter> InOwner) const
{
	const TWeakObjectPtr<UCCharacterStatComponent> stat = 
		Cast<UCCharacterStatComponent>(InOwner->GetComponentByClass(UCCharacterStatComponent::StaticClass()));
	CheckNull(stat);

	if (stat->GetCurStamina() < Stamina)
	{
		// TODO 스태미나 부족 시스템메시지, 카메라 떨림
		return;
	}

	stat->SetStaminaDamage(Stamina);
	stat->SetManaDamage(Mana);

	const TWeakObjectPtr<UCMovementComponent> movement = 
		YJJHelpers::GetComponent<UCMovementComponent>(InOwner.Get());

	if (!!movement.Get())
	{
		if (bFixedCamera)
			movement->EnableControlRotation();

		if (false == bCanMove)
			movement->Stop();
		else
			movement->Move();
	}

	if (!!Montage)
		InOwner->PlayAnimMontage(Montage, PlayRate);

	if (Sounds.Num() > 0)
		PlaySoundWave(InOwner);

	if (!!Effect)
		PlayEffect(InOwner->GetWorld(), InOwner->GetActorLocation(), InOwner->GetActorRotation());
}

void FActData::PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner) const
{
	const TWeakObjectPtr<UWorld> world = InOwner->GetWorld();
	CheckNull(world);

	const FVector location = InOwner->GetActorLocation();

	for (const auto& sound : Sounds)
	{
		CheckNull(sound);
		UGameplayStatics::SpawnSoundAtLocation(world.Get(), sound, location);
	}
}

void FActData::PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation) const
{
	CheckNull(Effect);

	FTransform transform;
	transform.SetLocation(EffectLocation);
	transform.SetScale3D(EffectScale);
	transform.AddToTranslation(InLocation);

	YJJHelpers::PlayEffect(InWorld.Get(), Effect, transform);
}

void FActData::PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation, const FRotator& InRotation) const
{
	CheckNull(Effect);

	FTransform transform;
	transform.SetLocation(InLocation + InRotation.RotateVector(EffectLocation));
	transform.SetScale3D(EffectScale);

	YJJHelpers::PlayEffect(InWorld.Get(), Effect, transform);
}

///////////////////////////////////////////////////////////////

void FHitData::SendDamage(
	const TWeakObjectPtr<ACCommonCharacter> InAttacker,
	const TWeakObjectPtr<AActor> InAttackCauser, 
	const TWeakObjectPtr<ACCommonCharacter> InOther) const
{
	FActDamageEvent damageEvent;
	damageEvent.HitData = *this;

	InOther->TakeDamage(Power, damageEvent, InAttacker->GetMyCurController().Get(), InAttackCauser.Get());
}

void FHitData::PlayMontage(const TWeakObjectPtr<ACCommonCharacter> InOwner) const
{
	if (!!Montage)
		InOwner->PlayAnimMontage(Montage, PlayRate);
}

void FHitData::PlayHitStop(const TWeakObjectPtr<UWorld> InWorld) const
{
	CheckTrue(FMath::IsNearlyZero(StopTime));

	TArray<TWeakObjectPtr<ACCommonCharacter>> characters;

	for (TWeakObjectPtr<AActor> actor : InWorld->GetCurrentLevel()->Actors)
	{
		const TWeakObjectPtr<ACCommonCharacter> character = Cast<ACCommonCharacter>(actor);

		if (!!character.Get())
		{
			character->CustomTimeDilation = 1e-3f;
			characters.Add(character);
		}
	}

	FTimerDelegate timerDelegate;
	timerDelegate.BindLambda([=]()
	{
		for (const TWeakObjectPtr<ACCommonCharacter> character : characters)
			character->CustomTimeDilation = 1;
	});

	FTimerHandle timerHandle;
	InWorld->GetTimerManager().SetTimer(timerHandle, timerDelegate, StopTime, false);
}

void FHitData::PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner) const
{
	for (const auto& sound : Sounds)
		CheckNull(sound);

	const TWeakObjectPtr<UWorld> world = InOwner->GetWorld();
	CheckNull(world);

	const FVector location = InOwner->GetActorLocation();

	for (const auto& sound : Sounds)
		UGameplayStatics::SpawnSoundAtLocation(world.Get(), sound, location);
}

void FHitData::PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation) const
{
	CheckNull(Effect);

	FTransform transform;
	transform.SetLocation(EffectLocation);
	transform.SetScale3D(EffectScale);
	transform.AddToTranslation(InLocation);

	YJJHelpers::PlayEffect(InWorld.Get(), Effect, transform);
}

void FHitData::PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation, const FRotator& InRotation) const
{
	CheckNull(Effect);

	FTransform transform;
	transform.SetLocation(InLocation + InRotation.RotateVector(EffectLocation));
	transform.SetScale3D(EffectScale);

	YJJHelpers::PlayEffect(InWorld.Get(), Effect, transform);
}
