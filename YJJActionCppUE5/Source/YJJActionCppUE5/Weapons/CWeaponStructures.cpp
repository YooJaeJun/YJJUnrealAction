#include "Weapons/CWeaponStructures.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CMovementComponent.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/CCamComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

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

void FEquipData::PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner) const
{
	CheckNull(Sound);

	const TWeakObjectPtr<UWorld> world = InOwner->GetWorld();
	CheckNull(world);

	const FVector location = InOwner->GetActorLocation();

	UGameplayStatics::SpawnSoundAtLocation(world.Get(), Sound, location);
}

void FDoActionData::Act(const TWeakObjectPtr<ACCommonCharacter> InOwner) const
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

	if ((false == FMath::IsNearlyZero(LaunchForward)) || (false == FMath::IsNearlyZero(LaunchUp)))
	{
		ACharacter* character = Cast<ACharacter>(InOwner.Get());
		if (IsValid(character))
		{
			UCharacterMovementComponent* moveComp = character->GetCharacterMovement();
			if (IsValid(moveComp))
			{
				const FVector impulse =
					character->GetActorForwardVector() * LaunchForward + FVector::UpVector * LaunchUp;
				moveComp->AddImpulse(impulse, true);
			}
		}
	}

	// bUseMotionTrail: 블루프린트의 모션 트레일 활성 노드 포팅은 이펙트/애니 노티파이 쪽과 묶는 것이 맞다(여기서는 데이터만 유지).
	(void)bUseMotionTrail;
	(void)AttackType;
	if (Tag.IsEmpty() == false)
	{
		// Tag 는 이펙트·어닐러틱스 식별용으로만 둔다. 조건 분기가 생기면 여기서 라우팅한다.
	}
}

void FDoActionData::PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner) const
{
	const TWeakObjectPtr<UWorld> world = InOwner->GetWorld();
	CheckNull(world);

	const FVector location = InOwner->GetActorLocation();

	CheckNull(Sound);
	UGameplayStatics::SpawnSoundAtLocation(world.Get(), Sound, location);
}

void FDoActionData::PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation) const
{
	CheckNull(Effect);

	FTransform transform;
	transform.SetLocation(EffectLocation);
	transform.SetScale3D(EffectScale);
	transform.AddToTranslation(InLocation);

	YJJHelpers::PlayEffect(InWorld.Get(), Effect, transform);
}

void FDoActionData::PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation, const FRotator& InRotation) const
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

	InOther->TakeDamage(Damage, damageEvent, InAttacker->GetMyCurController().Get(), InAttackCauser.Get());
}

void FHitData::PlayMontage(const TWeakObjectPtr<ACCommonCharacter> InOwner) const
{
	if (IsValid(Montage))
		InOwner->PlayAnimMontage(Montage, PlayRate);

	const TWeakObjectPtr<UCMovementComponent> movementComp =
		YJJHelpers::GetComponent<UCMovementComponent>(InOwner.Get());

	if (movementComp.IsValid())
	{
		if (false == bCanMove)
			movementComp->Stop();
		else
			movementComp->Move();
	}

	// CrowdControl 은 몽타주·스테이트 전환과 연동될 수 있어 별도 컴포넌트에서 소비하도록 데이터만 둔다.
	(void)CrowdControl;
}

void FHitData::PlayHitStop(const TWeakObjectPtr<UWorld> InWorld) const
{
	CheckTrue(FMath::IsNearlyZero(HitStop));

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
	InWorld->GetTimerManager().SetTimer(timerHandle, timerDelegate, HitStop, false);
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
