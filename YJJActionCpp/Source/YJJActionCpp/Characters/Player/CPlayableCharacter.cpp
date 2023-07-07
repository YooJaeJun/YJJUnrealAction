#include "CPlayableCharacter.h"
#include "Global.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/WidgetComponent.h"
#include "Characters/CAnimInstance_Human.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CMontagesComponent.h"
#include "Components/CWeaponComponent.h"
#include "Components/CZoomComponent.h"
#include "Components/CTargetingComponent.h"
#include "Components/CGameUIComponent.h"
#include "Commons/CGameMode.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/CCharacterStatComponent.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "Components/CRidingComponent.h"

ACPlayableCharacter::ACPlayableCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	YJJHelpers::CreateComponent<USpringArmComponent>(this, &SpringArm, "SpringArm", GetMesh());
	YJJHelpers::CreateComponent<UCameraComponent>(this, &Camera, "Camera", SpringArm);
	YJJHelpers::CreateActorComponent<UCWeaponComponent>(this, &WeaponComp, "WeaponComponent");
	YJJHelpers::CreateActorComponent<UCZoomComponent>(this, &ZoomComp, "ZoomComponent");
	YJJHelpers::CreateActorComponent<UCTargetingComponent>(this, &TargetingComp, "TargetingComponent");
	YJJHelpers::CreateActorComponent<UCGameUIComponent>(this, &GameUIComp, "GameUIComponent");

	USkeletalMesh* mesh;
	YJJHelpers::GetAsset<USkeletalMesh>(&mesh, "SkeletalMesh'/Game/Assets/Character/MercenaryWarrior/Meshes/SK_MercenaryWarrior_WithoutHelmet.SK_MercenaryWarrior_WithoutHelmet'");

	GetMesh()->SetSkeletalMesh(mesh);
	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	TSubclassOf<UCAnimInstance_Human> animInstance;
	YJJHelpers::GetClass<UCAnimInstance_Human>(&animInstance, "AnimBlueprint'/Game/Character/CABP_Human.CABP_Human_C'");
	GetMesh()->SetAnimClass(animInstance);

	if (!!SpringArm)
	{
		SpringArm->SetRelativeLocation(FVector(0, 0, 60));
		SpringArm->TargetArmLength = 280;
		SpringArm->bUsePawnControlRotation = true;
		SpringArm->bEnableCameraLag = true;
		SpringArm->bDoCollisionTest = false;
	}

	if (!!StateComp)
	{
		StateComp->SetIdleMode();
		StateComp->OnStateTypeChanged.AddUniqueDynamic(this, &ACPlayableCharacter::OnStateTypeChanged);
		StateComp->OnHitStateTypeChanged.AddUniqueDynamic(this, &ACPlayableCharacter::OnHitStateTypeChanged);
	}

	if (!!MovementComp)
	{
		MovementComp->SetSpeeds(Speeds);
		MovementComp->DisableControlRotation();
		MovementComp->UnFixCamera();
		MovementComp->SetFriction(2.0f, 2048.0f);
		MovementComp->SetJumpZ(700.0f);
	}
}

void ACPlayableCharacter::BeginPlay()
{
	Super::BeginPlay();

	GameMode = Cast<ACGameMode>(UGameplayStatics::GetGameMode(AActor::GetWorld()));

	if (!!MovementComp)
		MovementComp->SetSpeed(ESpeedType::Sprint);

	const TWeakObjectPtr<APlayerController> playerController = Cast<APlayerController>(MyCurController);
	if (!!playerController.Get())
	{
		playerController->PlayerCameraManager->ViewPitchMin = PitchRange.X;
		playerController->PlayerCameraManager->ViewPitchMax = PitchRange.Y;
	}

	const TWeakObjectPtr<UCUserWidget_HUD> hud = GameMode->GetHUD();
	if (hud.Get())
	{
		hud->SetChildren();

		const TWeakObjectPtr<UCUserWidget_PlayerInfo> playerInfo = hud->PlayerInfo;
		if (!!playerInfo.Get())
			playerInfo->BindStats(CharacterStatComp);
	}

	if (!!CharacterInfoComp)
		CharacterInfoComp->SetCharacterType(ECharacterType::Player);
}

void ACPlayableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACPlayableCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", MovementComp, &UCMovementComponent::InputAxis_MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", MovementComp, &UCMovementComponent::InputAxis_MoveRight);
	PlayerInputComponent->BindAxis("HorizontalLook", MovementComp, &UCMovementComponent::InputAxis_HorizontalLook);
	PlayerInputComponent->BindAxis("VerticalLook", MovementComp, &UCMovementComponent::InputAxis_VerticalLook);
	PlayerInputComponent->BindAxis("Zoom", ZoomComp, &UCZoomComponent::InputAxis_Zoom);

	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Pressed, MovementComp, &UCMovementComponent::InputAction_Walk);
	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Released, MovementComp, &UCMovementComponent::InputAction_Run);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, MovementComp, &UCMovementComponent::InputAction_Jump);
	PlayerInputComponent->BindAction("Avoid", EInputEvent::IE_Pressed, this, &ACPlayableCharacter::InputAction_Avoid);
	PlayerInputComponent->BindAction("Targeting", EInputEvent::IE_Pressed, TargetingComp, &UCTargetingComponent::InputAction_Targeting);
	PlayerInputComponent->BindAction("Menu", EInputEvent::IE_Pressed, GameUIComp, &UCGameUIComponent::InputAction_ActivateEquipMenu);
	PlayerInputComponent->BindAction("Menu", EInputEvent::IE_Released, GameUIComp, &UCGameUIComponent::InputAction_DeactivateEquipMenu);
	PlayerInputComponent->BindAction("Action", EInputEvent::IE_Pressed, WeaponComp, &UCWeaponComponent::InputAction_Act);
	PlayerInputComponent->BindAction("Skill_1", EInputEvent::IE_Pressed, WeaponComp, &UCWeaponComponent::InputAction_Skill_Pressed);
	PlayerInputComponent->BindAction("Skill_1", EInputEvent::IE_Released, WeaponComp, &UCWeaponComponent::InputAction_Skill_Released);
}

void ACPlayableCharacter::InputAction_Avoid()
{
	CheckFalse(StateComp->IsIdleMode());
	CheckFalse(MovementComp->CanMove());

	StateComp->SetAvoidMode();
}

void ACPlayableCharacter::Avoid()
{
	MontagesComp->PlayAvoidAnim();
}

void ACPlayableCharacter::Hit()
{
	CurHitType = Damage.Event.HitData.AttackType;

	Super::Hit();

	// Interaction
	const FHitData data = Damage.Event.HitData;

	if (StateComp->IsIdleMode())
		data.PlayMontage(this);

	data.PlayHitStop(GetWorld());
	data.PlaySoundWave(this);
	data.PlayEffect(GetWorld(), GetActorLocation(), GetActorRotation());

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
	else // if (CharacterStatComp->IsDead())
	{
		StateComp->SetDeadMode();
		return;
	}

	Damage.Attacker = nullptr;
	Damage.Causer = nullptr;
}

void ACPlayableCharacter::End_Avoid()
{
	StateComp->SetIdleMode();
}

void ACPlayableCharacter::End_Rise()
{
	StateComp->SetIdleMode();
}

void ACPlayableCharacter::End_Hit()
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

	StateComp->SetHitNoneMode();
}

void ACPlayableCharacter::OnStateTypeChanged(const EStateType InPrevType, const EStateType InNewType)
{
	switch (InNewType)
	{
	case EStateType::Avoid:
		Avoid();
		break;
	case EStateType::Rise:
		Rise();
		break;
	case EStateType::Land:
		Land();
		break;
	case EStateType::Dead:
		Dead();
		break;
	}
}

void ACPlayableCharacter::OnHitStateTypeChanged(const EHitType InPrevType, const EHitType InNewType)
{
	switch (InNewType)
	{
	case EHitType::None:
		break;
	default:
		Hit();
		break;
	}
}
