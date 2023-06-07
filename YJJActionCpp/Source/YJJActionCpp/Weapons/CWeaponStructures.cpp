#include "Weapons/CWeaponStructures.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/CCharacterStatComponent.h"

void FEquipmentData::PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner)
{
	for (const auto& sound : Sounds)
		CheckNull(sound);

	TWeakObjectPtr<UWorld> world = InOwner->GetWorld();
	FVector location = InOwner->GetActorLocation();

	for (const auto& sound : Sounds)
		UGameplayStatics::SpawnSoundAtLocation(world.Get(), sound, location);
}

void FActData::Act(const TWeakObjectPtr<ACCommonCharacter> InOwner)
{
	TWeakObjectPtr<UCCharacterStatComponent> stat = 
		Cast<UCCharacterStatComponent>(InOwner->GetComponentByClass(UCCharacterStatComponent::StaticClass()));

	CheckNull(stat.Get());

	if (stat->GetCurStamina() < Stamina)
	{
		// TODO 스태미나 부족 시스템메시지, 카메라 떨림
		return;
	}

	stat->SetStaminaDamage(Stamina);
	stat->SetManaDamage(Mana);

	TWeakObjectPtr<UCMovementComponent> movement = CHelpers::GetComponent<UCMovementComponent>(InOwner.Get());

	if (!!movement.Get())
	{
		if (bFixedCamera)
			movement->EnableControlRotation();

		if (false == bCanMove)
			movement->Stop();
	}

	if (!!Montage)
		InOwner->PlayAnimMontage(Montage, PlayRate);

	if (Sounds.Num() > 0)
		PlaySoundWave(InOwner);

	if (!!Effect)
		PlayEffect(InOwner->GetWorld(), InOwner->GetActorLocation(), InOwner->GetActorRotation());
}

void FActData::PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner)
{
	TWeakObjectPtr<UWorld> world = InOwner->GetWorld();
	FVector location = InOwner->GetActorLocation();

	for (const auto& sound : Sounds)
	{
		CheckNull(sound);
		UGameplayStatics::SpawnSoundAtLocation(world.Get(), sound, location);
	}
}

void FActData::PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation)
{
	CheckNull(Effect);

	FTransform transform;
	transform.SetLocation(EffectLocation);
	transform.SetScale3D(EffectScale);
	transform.AddToTranslation(InLocation);

	CHelpers::PlayEffect(InWorld.Get(), Effect, transform);
}

void FActData::PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation, const FRotator& InRotation)
{
	CheckNull(Effect);

	FTransform transform;
	transform.SetLocation(InLocation + InRotation.RotateVector(EffectLocation));
	transform.SetScale3D(EffectScale);

	CHelpers::PlayEffect(InWorld.Get(), Effect, transform);
}

///////////////////////////////////////////////////////////////

void FHitData::SendDamage(const TWeakObjectPtr<ACCommonCharacter> InAttacker,
	const TWeakObjectPtr<AActor> InAttackCauser, const TWeakObjectPtr<ACCommonCharacter> InOther)
{
	FActDamageEvent e;
	e.HitData = this;

	InOther->TakeDamage(Power, e, InAttacker->GetMyCurController().Get(), InAttackCauser.Get());
}

void FHitData::PlayMontage(const TWeakObjectPtr<ACCommonCharacter> InOwner)
{
	if (!!Montage)
		InOwner->PlayAnimMontage(Montage, PlayRate);
}

void FHitData::PlayHitStop(const TWeakObjectPtr<UWorld> InWorld)
{
	CheckTrue(FMath::IsNearlyZero(StopTime));

	TArray<ACCommonCharacter*> characters;
	for (AActor* actor : InWorld->GetCurrentLevel()->Actors)
	{
		ACCommonCharacter* character = Cast<ACCommonCharacter>(actor);

		if (!!character)
		{
			character->CustomTimeDilation = 1e-3f;
			characters.Add(character);
		}
	}

	FTimerDelegate timerDelegate;
	timerDelegate.BindLambda([=]()
	{
		for (ACCommonCharacter* character : characters)
			character->CustomTimeDilation = 1;
	});

	FTimerHandle timerHandle;
	InWorld->GetTimerManager().SetTimer(timerHandle, timerDelegate, StopTime, false);
}

void FHitData::PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner)
{
	for (const auto& sound : Sounds)
		CheckNull(sound);

	TWeakObjectPtr<UWorld> world = InOwner->GetWorld();
	FVector location = InOwner->GetActorLocation();

	for (const auto& sound : Sounds)
		UGameplayStatics::SpawnSoundAtLocation(world.Get(), sound, location);
}

void FHitData::PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation)
{
	CheckNull(Effect);

	FTransform transform;
	transform.SetLocation(EffectLocation);
	transform.SetScale3D(EffectScale);
	transform.AddToTranslation(InLocation);

	CHelpers::PlayEffect(InWorld.Get(), Effect, transform);
}

void FHitData::PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation, const FRotator& InRotation)
{
	CheckNull(Effect);

	FTransform transform;
	transform.SetLocation(InLocation + InRotation.RotateVector(EffectLocation));
	transform.SetScale3D(EffectScale);

	CHelpers::PlayEffect(InWorld.Get(), Effect, transform);
}
