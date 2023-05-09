#include "Weapons/CWeaponStructures.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Animation/AnimMontage.h"

void FActData::Act(ACCommonCharacter* InOwner)
{
	TWeakObjectPtr<UCMovementComponent> movement = CHelpers::GetComponent<UCMovementComponent>(InOwner);

	if (!!movement.Get())
	{
		if (bFixedCamera)
			movement->EnableControlRotation();

		if (false == bCanMove)
			movement->Stop();
	}

	if (!!Montage)
		InOwner->PlayAnimMontage(Montage, PlayRate);
}

///////////////////////////////////////////////////////////////

void FHitData::SendDamage(ACCommonCharacter* InAttacker, 
	AActor* InAttackCauser, ACCommonCharacter* InOther)
{
	FActDamageEvent e;
	e.HitData = this;

	InOther->TakeDamage(Power, e, 
		InAttacker->GetController(), InAttackCauser);
}

void FHitData::PlayMontage(ACCommonCharacter* InOwner)
{
	if (!!Montage)
		InOwner->PlayAnimMontage(Montage, PlayRate);
}

void FHitData::PlayHitStop(UWorld* InWorld)
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
}

void FHitData::PlaySoundWave(ACCommonCharacter* InOwner)
{
	CheckNull(Sound);

	UWorld* world = InOwner->GetWorld();
	FVector location = InOwner->GetActorLocation();

	UGameplayStatics::SpawnSoundAtLocation(world, Sound, location);
}