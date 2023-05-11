#include "CCommonCharacter.h"
#include "Global.h"
#include "Components/CapsuleComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CMontagesComponent.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/CCharacterStatComponent.h"
#include "Weapons/CWeaponStructures.h"

ACCommonCharacter::ACCommonCharacter()
{
	CHelpers::CreateActorComponent<UCStateComponent>(this, &StateComp, "StateComponent");
	CHelpers::CreateActorComponent<UCMovementComponent>(this, &MovementComp, "MovementComponent");
	CHelpers::CreateActorComponent<UCMontagesComponent>(this, &MontagesComp, "MontagesComponent");
	CHelpers::CreateActorComponent<UCCharacterInfoComponent>(this, &CharacterInfoComp, "CharacterInfoComponent");
	CHelpers::CreateActorComponent<UCCharacterStatComponent>(this, &CharacterStatComp, "CharacterStatComponent");

	MyCurController = GetController();
}

void ACCommonCharacter::BeginPlay()
{
	Super::BeginPlay();
}

float ACCommonCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	float damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Damage.Power = damage;
	Damage.Character = Cast<ACCommonCharacter>(EventInstigator->GetPawn());
	Damage.Causer = DamageCauser;

	// UObject가 아니기 때문에 Cast 사용 불가
	// 일반 캐스팅 사용 시, 부모와 자식의 크기가 다르기 때문에
	// '포인터'로 통일해 크기 동일하게 맞춰줌
	// Damage.Event = (FActDamageEvent*)&DamageEvent;
	Damage.Event = (FActDamageEvent*)&DamageEvent;

	StateComp->SetHitMode();

	return damage;
}

void ACCommonCharacter::Rise()
{
	MontagesComp->PlayRiseAnim();
}

void ACCommonCharacter::Hit()
{
	// Apply Damage
	{
		CharacterStatComp->Damage(Damage.Power);
		Damage.Power = 0;
	}

	// Damage
	if (!!Damage.Event &&
		!!Damage.Event->HitData)
	{
		FHitData* data = Damage.Event->HitData;

		data->PlayMontage(this);
		data->PlayHitStop(GetWorld());
		data->PlaySoundWave(this);
		data->PlayEffect(GetWorld(), GetActorLocation(), GetActorRotation());

		if (false == CharacterStatComp->IsDead())
		{
			FVector start = GetActorLocation();
			FVector target = Damage.Character->GetActorLocation();
			FVector direction = target - start;
			direction.Normalize();

			LaunchCharacter(-direction * data->Launch, false, false);
			SetActorRotation(UKismetMathLibrary::FindLookAtRotation(start, target));
		}
	}

	if (CharacterStatComp->IsDead())
	{
		StateComp->SetDeadMode();
		return;
	}

	Damage.Character = nullptr;
	Damage.Causer = nullptr;
	Damage.Event = nullptr;
}

void ACCommonCharacter::Dead()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MontagesComp->PlayDeadAnim();
}

void ACCommonCharacter::End_Hit()
{
	ICInterface_CharacterAnim::End_Hit();
}

void ACCommonCharacter::End_Dead()
{
	Destroy();
}

void ACCommonCharacter::End_Rise()
{
	StateComp->SetIdleMode();
}

void ACCommonCharacter::RestoreColor()
{
	ChangeColor(this, OriginColor);

	GetWorld()->GetTimerManager().ClearTimer(RestoreColor_TimerHandle);
}