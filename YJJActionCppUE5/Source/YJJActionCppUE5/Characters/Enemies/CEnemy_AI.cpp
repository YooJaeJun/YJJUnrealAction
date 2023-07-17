#include "Characters/Enemies/CEnemy_AI.h"

#include <string>

#include "Global.h"
#include "Characters/CAnimInstance_Human.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CMontagesComponent.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/CWeaponComponent.h"
#include "Components/WidgetComponent.h"
#include "Weapons/CWeaponStructures.h"

ACEnemy_AI::ACEnemy_AI()
{
	PrimaryActorTick.bCanEverTick = true;

	YJJHelpers::CreateActorComponent<UCWeaponComponent>(this, &WeaponComp, "Weapon");

	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	TObjectPtr<USkeletalMesh> mesh;
	YJJHelpers::GetAsset<USkeletalMesh>(&mesh, "SkeletalMesh'/Game/Assets/Enemies/Skeleton_archer/mesh/SK_Skeleton_archer.SK_Skeleton_archer'");
	GetMesh()->SetSkeletalMesh(mesh);

	TSubclassOf<UCAnimInstance_Human> animInstance;
	YJJHelpers::GetClass<UCAnimInstance_Human>(&animInstance, "AnimBlueprint'/Game/Character/CABP_Human.CABP_Human_C'");
	GetMesh()->SetAnimClass(animInstance);

	GetCharacterMovement()->RotationRate = FRotator(0, 720, 0);
	
	if (IsValid(StateComp))
	{
		StateComp->OnStateTypeChanged.AddUniqueDynamic(this, &ACEnemy_AI::OnStateTypeChanged);
		StateComp->OnHitStateTypeChanged.AddUniqueDynamic(this, &ACEnemy_AI::OnHitStateTypeChanged);
	}

	if (IsValid(MovementComp))
	{
		MovementComp->SetSpeeds(Speeds);
		MovementComp->InputAction_Run();
	}

	if (IsValid(InfoWidgetComp))
		InfoWidgetComp->SetVisibility(true);
}

void ACEnemy_AI::BeginPlay()
{
	Super::BeginPlay();

	Create_DynamicMaterial(this);
	ChangeColor(this, OriginColor);

	if (IsValid(CharacterInfoComp))
		if (CharacterInfoComp->GetCharacterType() == 0)
			CharacterInfoComp->SetCharacterType(CECharacterType::Enemy_1);

	if (IsValid(CharacterStatComp))
		CharacterStatComp->SetAttackRange(250.0f);
}

void ACEnemy_AI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACEnemy_AI::OnStateTypeChanged(CEStateType InPrevType, CEStateType InNewType)
{
	switch (InNewType)
	{
	case CEStateType::Rise:
		Rise();
		break;
	case CEStateType::Dead:
		Dead();
		break;
	}
}

void ACEnemy_AI::Hit()
{
	// Change Color
	{
		ChangeColor(this, FLinearColor::Red);

		FTimerDelegate timerDelegate;
		timerDelegate.BindUFunction(this, "RestoreColor");

		GetWorld()->GetTimerManager().SetTimer(RestoreColor_TimerHandle, timerDelegate, 0.2f, false);
	}

	CurHitType = Damage.Event.HitData.AttackType;

	Super::Hit();

	// Interaction
	const FHitData data = Damage.Event.HitData;
	
	if (StateComp->IsIdleMode())
		data.PlayMontage(this);

	data.PlayHitStop(GetWorld());
	data.PlaySoundWave(this);
	data.PlayEffect(GetWorld(), Damage.Event.HitData.EffectLocation, GetActorRotation());

	if (false == CharacterStatComp->IsDead())
	{
		const FVector start = GetActorLocation();

		CheckNull(Damage.Attacker);
		const FVector target = Damage.Attacker->GetActorLocation();

		FVector direction = target - start;
		direction.Normalize();

		LaunchCharacter(-direction * data.Launch, false, false);
		SetActorRotation(UKismetMathLibrary::FindLookAtRotation(start, target));
	}

	if (CharacterStatComp->IsDead())
	{
		StateComp->SetDeadMode();
		return;
	}

	InfoWidgetComp->SetVisibility(true);

	Damage.Attacker = nullptr;
	Damage.Causer = nullptr;


	// Cancel Act
	CheckNull(WeaponComp);
	WeaponComp->CancelAct();
}

void ACEnemy_AI::End_Hit()
{
	Super::End_Hit();

	switch (CurHitType)
	{
	case CEHitType::Knockback:
	case CEHitType::Down:
		StateComp->SetRiseMode();
		break;
	case CEHitType::Air:
	case CEHitType::Fly:
		StateComp->SetFallMode();
		break;
	default:
		StateComp->SetIdleMode();
		StateComp->SetHitNoneMode();
		break;
	}
}

void ACEnemy_AI::End_Rise()
{
	StateComp->SetIdleMode();
	StateComp->SetHitNoneMode();
}

void ACEnemy_AI::OnHitStateTypeChanged(const CEHitType InPrevType, const CEHitType InNewType)
{
	switch (InNewType)
	{
	case CEHitType::None:
		break;
	default:
		Hit();
	}
}