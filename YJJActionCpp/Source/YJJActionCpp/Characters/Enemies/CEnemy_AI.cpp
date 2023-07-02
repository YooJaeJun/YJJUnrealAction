#include "Characters/Enemies/CEnemy_AI.h"
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

	CHelpers::CreateActorComponent<UCWeaponComponent>(this, &WeaponComp, "Weapon");

	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	USkeletalMesh* mesh;
	CHelpers::GetAsset<USkeletalMesh>(&mesh, "SkeletalMesh'/Game/Assets/Enemies/Skeleton_archer/mesh/SK_Skeleton_archer.SK_Skeleton_archer'");
	GetMesh()->SetSkeletalMesh(mesh);

	TSubclassOf<UCAnimInstance_Human> animInstance;
	CHelpers::GetClass<UCAnimInstance_Human>(&animInstance, "AnimBlueprint'/Game/Character/CABP_Human.CABP_Human_C'");
	GetMesh()->SetAnimClass(animInstance);

	GetCharacterMovement()->RotationRate = FRotator(0, 720, 0);
	
	if (!!StateComp)
		StateComp->OnStateTypeChanged.AddUniqueDynamic(this, &ACEnemy_AI::OnStateTypeChanged);

	if (!!MovementComp)
	{
		MovementComp->SetSpeeds(Speeds);
		MovementComp->InputAction_Run();
	}

	if (!!InfoWidgetComp)
		InfoWidgetComp->SetVisibility(true);
}

void ACEnemy_AI::BeginPlay()
{
	Super::BeginPlay();

	Create_DynamicMaterial(this);
	ChangeColor(this, OriginColor);

	if (!!CharacterInfoComp)
		if (CharacterInfoComp->GetCharacterType() == 0)
			CharacterInfoComp->SetCharacterType(ECharacterType::Enemy_1);

	if (!!CharacterStatComp)
		CharacterStatComp->SetAttackRange(250.0f);
}

void ACEnemy_AI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACEnemy_AI::OnStateTypeChanged(EStateType InPrevType, EStateType InNewType)
{
	switch (InNewType)
	{
	case EStateType::Rise:
		Rise();
		break;
	case EStateType::Hit:
		Hit();
		break;
	case EStateType::Dead:
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

	// Cancel Hit
	CheckNull(WeaponComp);
	WeaponComp->CancelAct();
}

void ACEnemy_AI::End_Hit()
{
	Super::End_Hit();

	switch (CurHitType)
	{
	case EHitType::Knockback:
		StateComp->SetRiseMode();
		break;
	case EHitType::Air:
	case EHitType::Fly:
		StateComp->SetFallMode();
		break;
	default:
		StateComp->SetIdleMode();
		break;
	}
}
