#include "Characters/Enemies/CEnemy.h"
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
#include "Widgets/Enemies/CUserWidget_EnemyInfo.h"
#include "Widgets/Weapons/CUserWidget_EquipMenu.h"

ACEnemy::ACEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	YJJHelpers::CreateComponent<USceneComponent>(this, &InfoPoint, "InfoPoint", GetMesh());
	YJJHelpers::CreateComponent<UWidgetComponent>(this, &InfoWidgetComp, "InfoWidgetComp", InfoPoint);
	YJJHelpers::GetClass<UCUserWidget_EnemyInfo>(&InfoWidget, "WidgetBlueprint'/Game/Widgets/Enemy/CWB_EnemyBar.CWB_EnemyBar_C'");
	YJJHelpers::CreateActorComponent<UCWeaponComponent>(this, &WeaponComp, "Weapon");

	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	USkeletalMesh* mesh;
	YJJHelpers::GetAsset<USkeletalMesh>(&mesh, "SkeletalMesh'/Game/Assets/Enemies/Skeleton_archer/mesh/SK_Skeleton_archer.SK_Skeleton_archer'");
	GetMesh()->SetSkeletalMesh(mesh);

	TSubclassOf<UCAnimInstance_Human> animInstance;
	YJJHelpers::GetClass<UCAnimInstance_Human>(&animInstance, "AnimBlueprint'/Game/Character/CABP_Human.CABP_Human_C'");
	GetMesh()->SetAnimClass(animInstance);

	GetCharacterMovement()->RotationRate = FRotator(0, 720, 0);

	StateComp->OnStateTypeChanged.AddUniqueDynamic(this, &ACEnemy::OnStateTypeChanged);
	StateComp->OnHitStateTypeChanged.AddUniqueDynamic(this, &ACEnemy::OnHitStateTypeChanged);

	MovementComp->SetSpeeds(Speeds);
	MovementComp->InputAction_Run();

	InfoWidgetComp->SetWidgetClass(InfoWidget);
	InfoWidgetComp->SetRelativeLocation(FVector(0, 0, 220));
	InfoWidgetComp->SetDrawSize(FVector2D(120, 0));
	InfoWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
}

void ACEnemy::BeginPlay()
{
	Super::BeginPlay();

	Create_DynamicMaterial(this);
	ChangeColor(this, OriginColor);

	if (IsValid(CharacterInfoComp))
		if (CharacterInfoComp->GetCharacterType() == 0)
			CharacterInfoComp->SetCharacterType(static_cast<CECharacterType>(CharacterInfoComp->CurType));

	if (IsValid(CharacterStatComp))
	{
		CharacterStatComp->SetAttackRange(250);
		CharacterStatComp->SetMaxHp(100);
		CharacterStatComp->SetHp(100);
	}

	if (IsValid(InfoWidgetComp))
	{
		InfoBar = Cast<UCUserWidget_EnemyInfo>(InfoWidgetComp->GetUserWidgetObject());
		InfoBar->BindChildren(CharacterInfoComp, CharacterStatComp);
		InfoWidgetComp->SetVisibility(false);
	}
}

void ACEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (InfoBar.IsValid())
		InfoBar->UpdateEnemyBarWidget();
}

void ACEnemy::OnStateTypeChanged(CEStateType InPrevType, CEStateType InNewType)
{
	switch (InNewType)
	{
	case CEStateType::Rise:
		Rise();
		break;
	case CEStateType::Dead:
		Dead();
		break;
	default: 
		break;
	}
}

void ACEnemy::OnHitStateTypeChanged(const CEHitType InPrevType, const CEHitType InNewType)
{
	switch (InNewType)
	{
	case CEHitType::None:
		break;
	default:
		Hit();
	}
}

void ACEnemy::Hit()
{
	// Info Bar
	{
		InfoWidgetComp->SetVisibility(true);

		FTimerHandle infoBarTimerHandle;
		FTimerDelegate infoBarDelegate;
		infoBarDelegate.BindLambda([this]()
		{
			InfoWidgetComp->SetVisibility(false);
		});
		GetWorld()->GetTimerManager().SetTimer(
			infoBarTimerHandle,
			infoBarDelegate,
			3,
			false);
	}

	// Change Color
	{
		ChangeColor(this, FLinearColor::Red);

		FTimerDelegate timerDelegate;
		timerDelegate.BindUFunction(this, "RestoreColor");

		GetWorld()->GetTimerManager().SetTimer(
			RestoreColorTimerHandle, 
			timerDelegate, 
			0.2f, 
			false);
	}

	// AttackType
	CurHitType = Damage.Event.HitData.AttackType;

	Super::Hit();

	// Interaction
	const FHitData data = Damage.Event.HitData;

	if (StateComp->IsIdleMode())
		data.PlayMontage(this);

	data.PlayHitStop(GetWorld());
	data.PlaySoundWave(this);
	data.PlayEffect(GetWorld(), GetActorLocation() + Damage.Event.HitData.EffectLocation, GetActorRotation());

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

void ACEnemy::RestoreColor()
{
	ChangeColor(this, OriginColor);

	GetWorld()->GetTimerManager().ClearTimer(RestoreColorTimerHandle);
}

void ACEnemy::End_Hit()
{
	Super::End_Hit();

	switch (CurHitType)
	{
	case CEHitType::Knockback:
	case CEHitType::Down:
	{
		StateComp->SetRiseMode();

		//FTimerHandle RiseDelayTimerHandle;
		//const float rand = UKismetMathLibrary::RandomFloatInRange(1.0f, 3.0f);

		//GetWorldTimerManager().SetTimer(
		//	RiseDelayTimerHandle,
		//	[this]() -> void {
		//	}, rand, false, 0.5f);

		break;
	}
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

void ACEnemy::End_Rise()
{
	Super::End_Rise();

	StateComp->SetHitNoneMode();
}