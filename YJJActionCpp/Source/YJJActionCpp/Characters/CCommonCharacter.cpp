#include "CCommonCharacter.h"
#include "Global.h"
#include "Components/CapsuleComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CMontagesComponent.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/CCharacterStatComponent.h"
#include "Weapons/CWeaponStructures.h"
#include "UMG/Public/Blueprint/WidgetLayoutLibrary.h"
#include "Components/WidgetComponent.h"
#include "Widgets/CUserWidget_Custom.h"
#include "Components/SceneComponent.h"

ACCommonCharacter::ACCommonCharacter()
{
	CHelpers::CreateActorComponent<UCStateComponent>(this, &StateComp, "StateComponent");
	CHelpers::CreateActorComponent<UCMovementComponent>(this, &MovementComp, "MovementComponent");
	CHelpers::CreateActorComponent<UCMontagesComponent>(this, &MontagesComp, "MontagesComponent");
	CHelpers::CreateActorComponent<UCCharacterInfoComponent>(this, &CharacterInfoComp, "CharacterInfoComponent");
	CHelpers::CreateActorComponent<UCCharacterStatComponent>(this, &CharacterStatComp, "CharacterStatComponent");
	CHelpers::CreateComponent<USceneComponent>(this, &TargetingPoint, "TargetingPoint", GetMesh());
	CHelpers::CreateComponent<UWidgetComponent>(this, &TargetingWidgetComp, "TargetingWidgetComp", TargetingPoint);
	CHelpers::GetClass<UCUserWidget_Custom>(&TargetingWidget, "WidgetBlueprint'/Game/Widgets/Interaction/CWB_Targeting.CWB_Targeting_C'");

	CharacterStatComp->OnHpIsZero.AddUObject(this, &ACCommonCharacter::Dead);

	if (!!TargetingWidgetComp)
	{
		if (!!TargetingWidget)
			TargetingWidgetComp->SetWidgetClass(TargetingWidget);

		TargetingWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
		TargetingWidgetComp->SetVisibility(false);
	}
}

void ACCommonCharacter::BeginPlay()
{
	Super::BeginPlay();

	MyCurController = GetController();

	CHelpers::GetAssetDynamic<USoundBase>(&LandSound,
		TEXT("SoundWave'/Game/Assets/Sounds/Action/Sway_2.Sway_2'"));

	if (!!TargetingPoint)
		TargetingPoint->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Targeting");
}

void ACCommonCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bTickLerpForTarget)
	{
		const FRotator start = MyCurController->GetControlRotation();
		const FRotator target = TargetRotator;

		MyCurController->SetControlRotation(
			FMath::RInterpTo(start, target, GetWorld()->DeltaTimeSeconds, 5.0f));

		if (false == UKismetMathLibrary::EqualEqual_Vector2DVector2D(
			UWidgetLayoutLibrary::GetMousePositionOnPlatform(), GetMousePos(), 1.0f))
			bTickLerpForTarget = false;

		if (UKismetMathLibrary::EqualEqual_RotatorRotator(start, target, 1.0f))
			bTickLerpForTarget = false;
	}
}

void ACCommonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Interact", EInputEvent::IE_Pressed, this, &ACCommonCharacter::InputAction_Interact);
}

void ACCommonCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	StateComp->GoBack();
}

float ACCommonCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, 
                                    AController* EventInstigator, AActor* DamageCauser)
{
	const float damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Damage.Power = damage;
	Damage.Attacker = Cast<ACCommonCharacter>(EventInstigator->GetPawn());
	Damage.Causer = DamageCauser;

	// UObject가 아니기 때문에 Cast 사용 불가
	// 일반 캐스팅 사용 시, 부모와 자식의 크기가 다르기 때문에
	// '포인터'로 통일해 크기 동일하게 맞춰줌
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

	// Interaction
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
			const FVector start = GetActorLocation();
			const FVector target = Damage.Attacker->GetActorLocation();
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

	Damage.Attacker = nullptr;
	Damage.Causer = nullptr;
	Damage.Event = nullptr;
}

void ACCommonCharacter::Dead()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MontagesComp->PlayDeadAnim();

	FTimerHandle DestroyDelayTimerHandle;
	GetWorldTimerManager().SetTimer(DestroyDelayTimerHandle, [this]() -> void {
		Destroy();
		}, 1.5f, false, 1.5f);
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

void ACCommonCharacter::SetMyCurController(const TWeakObjectPtr<AController> InController)
{
	MyCurController = InController;
}

void ACCommonCharacter::SetTickLerp(FRotator InRotator)
{
	TargetRotator = InRotator;
}

void ACCommonCharacter::TogglebTickLerpForTarget()
{
	bTickLerpForTarget = !bTickLerpForTarget;
}

void ACCommonCharacter::SetMousePos(const FVector2D InPos)
{
	MousePos = InPos;
}

void ACCommonCharacter::SetInteractor(ACCommonCharacter* InCharacter)
{
	Interactor = InCharacter;
}

void ACCommonCharacter::InputAction_Interact()
{
	if (!!Interactor)
	{
		if (OnMount.IsBound())
			OnMount.Broadcast(this);
	}
	else
	{
		if (OnUnmount.IsBound())
			OnUnmount.Broadcast();
	}
}
