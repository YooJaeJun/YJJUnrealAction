#include "Weapons/CWeaponStructures.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/CCamComponent.h"

namespace HitStopDetail
{
	static void RestoreTimeDilation(TArray<TWeakObjectPtr<ACCommonCharacter>> Characters)
	{
		for (const TWeakObjectPtr<ACCommonCharacter>& character : Characters)
		{
			if (character.IsValid())
			{
				character->CustomTimeDilation = 1.0f;
			}
		}
	}
}

void FEquipmentData::PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner) const
{
	CheckNull(Sound);

	const TWeakObjectPtr<UWorld> world = InOwner->GetWorld();
	CheckNull(world);

	const FVector location = InOwner->GetActorLocation();

	UGameplayStatics::SpawnSoundAtLocation(world.Get(), Sound, location);
}

void FActData::Act(const TWeakObjectPtr<ACCommonCharacter> InOwner) const
{
	const TWeakObjectPtr<UCCharacterStatComponent> stat = 
		Cast<UCCharacterStatComponent>(InOwner->GetComponentByClass(UCCharacterStatComponent::StaticClass()));
	CheckNull(stat);

	if (stat->GetCurStamina() < Stamina)
	{
		// TODO: 스태미나가 부족할 때 UI나 카메라 연출로 피드백을 추가한다.
		return;
	}

	stat->SetStaminaDamage(Stamina);
	stat->SetManaDamage(Mana);

	const TWeakObjectPtr<UCMovementComponent> movementComp =
		YJJHelpers::GetComponent<UCMovementComponent>(InOwner.Get());

	if (movementComp.IsValid())
	{
		if (false == bCanMove)
			movementComp->Stop();
		else
			movementComp->Move();
	}

	const TWeakObjectPtr<UCCamComponent> camComp =
		YJJHelpers::GetComponent<UCCamComponent>(InOwner.Get());

	if (camComp.IsValid() && true == bFixedCamera)
		camComp->EnableControlRotation();

	if (IsValid(Montage))
		InOwner->PlayAnimMontage(Montage, PlayRate);

	if (IsValid(Sound))
		PlaySoundWave(InOwner);

	if (IsValid(Effect))
		PlayEffect(InOwner->GetWorld(), InOwner->GetActorLocation(), InOwner->GetActorRotation());
}

void FActData::PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner) const
{
	const TWeakObjectPtr<UWorld> world = InOwner->GetWorld();
	CheckNull(world);

	const FVector location = InOwner->GetActorLocation();

	CheckNull(Sound);
	UGameplayStatics::SpawnSoundAtLocation(world.Get(), Sound, location);
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
	if (IsValid(Montage))
		InOwner->PlayAnimMontage(Montage, PlayRate);
}

void FHitData::PlayHitStop(const TWeakObjectPtr<UWorld> InWorld) const
{
	CheckTrue(FMath::IsNearlyZero(StopTime));

	TArray<TWeakObjectPtr<ACCommonCharacter>> characters;

	for (const TObjectPtr<AActor>& actor : InWorld->GetCurrentLevel()->Actors)
	{
		const TObjectPtr<ACCommonCharacter> character = Cast<ACCommonCharacter>(actor.Get());

		if (IsValid(character.Get()))
		{
			character->CustomTimeDilation = 1e-3f;
			characters.Add(TWeakObjectPtr<ACCommonCharacter>(character.Get()));
		}
	}

	FTimerDelegate timerDelegate = FTimerDelegate::CreateStatic(
		&HitStopDetail::RestoreTimeDilation,
		MoveTemp(characters));

	FTimerHandle timerHandle;
	InWorld->GetTimerManager().SetTimer(timerHandle, timerDelegate, StopTime, false);
}

void FHitData::PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner) const
{
	CheckNull(Sound);

	const TWeakObjectPtr<UWorld> world = InOwner->GetWorld();
	CheckNull(world);

	const FVector location = InOwner->GetActorLocation();

	UGameplayStatics::SpawnSoundAtLocation(world.Get(), Sound, location);
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
