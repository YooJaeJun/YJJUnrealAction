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
#include "Components/CCamComponent.h"
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
	YJJHelpers::CreateActorComponent<UCCamComponent>(this, &CamComp, "CamComponent");
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

	if (IsValid(SpringArm))
	{
		SpringArm->SetRelativeLocation(FVector(0, 0, 60));
		SpringArm->TargetArmLength = 280;
		SpringArm->bUsePawnControlRotation = true;
		SpringArm->bEnableCameraLag = true;
		SpringArm->bDoCollisionTest = false;
	}

	if (IsValid(StateComp))
	{
		StateComp->SetIdleMode();
		StateComp->OnStateTypeChanged.AddUniqueDynamic(this, &ACPlayableCharacter::OnStateTypeChanged);
		StateComp->OnHitStateTypeChanged.AddUniqueDynamic(this, &ACPlayableCharacter::OnHitStateTypeChanged);
	}

	if (IsValid(MovementComp))
	{
		MovementComp->SetSpeeds(Speeds);
		MovementComp->SetFriction(2.0f, 2048.0f);
		MovementComp->SetJumpZ(700.0f);
		MovementComp->SetGravity(2.5f);
	}

	if (IsValid(CamComp))
	{
		CamComp->DisableControlRotation();
		CamComp->DisableFixedCamera();

		if (IsValid(MovementComp))
		{
			if (CamComp->OnEnableTopViewCam.IsBound())
				CamComp->OnEnableTopViewCam.AddDynamic(MovementComp, &UCMovementComponent::OnEnableTopViewCam);

			if (CamComp->OnDisableTopViewCam.IsBound())
				CamComp->OnDisableTopViewCam.AddDynamic(MovementComp, &UCMovementComponent::OnDisableTopViewCam);
		}
	}
}

void ACPlayableCharacter::BeginPlay()
{
	Super::BeginPlay();

	GameMode = Cast<ACGameMode>(UGameplayStatics::GetGameMode(AActor::GetWorld()));

	if (IsValid(MovementComp))
		MovementComp->SetSpeed(CESpeedType::Sprint);

	const TWeakObjectPtr<APlayerController> playerController = Cast<APlayerController>(GetMyCurController());
	if (playerController.IsValid())
	{
		playerController->PlayerCameraManager->ViewPitchMin = PitchRange.X;
		playerController->PlayerCameraManager->ViewPitchMax = PitchRange.Y;
	}

	const TWeakObjectPtr<UCUserWidget_HUD> hud = GameMode->GetHUD();
	if (hud.Get())
	{
		hud->SetChildren();

		const TWeakObjectPtr<UCUserWidget_PlayerInfo> playerInfo = hud->PlayerInfo;
		if (playerInfo.IsValid())
			playerInfo->BindStats(CharacterStatComp);
	}

	if (IsValid(CharacterInfoComp))
		CharacterInfoComp->SetCharacterType(CECharacterType::Player);
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
	PlayerInputComponent->BindAxis("HorizontalLook", CamComp, &UCCamComponent::InputAxis_HorizontalLook);
	PlayerInputComponent->BindAxis("VerticalLook", CamComp, &UCCamComponent::InputAxis_VerticalLook);
	PlayerInputComponent->BindAxis("Zoom", CamComp, &UCCamComponent::InputAxis_Zoom);

	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Pressed, MovementComp, &UCMovementComponent::InputAction_Walk);
	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Released, MovementComp, &UCMovementComponent::InputAction_Run);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, MovementComp, &UCMovementComponent::InputAction_Jump);
	PlayerInputComponent->BindAction("Avoid", EInputEvent::IE_Pressed, this, &ACPlayableCharacter::InputAction_Avoid);
	PlayerInputComponent->BindAction("Targeting", EInputEvent::IE_Pressed, TargetingComp, &UCTargetingComponent::InputAction_Targeting);
	PlayerInputComponent->BindAction("Menu", EInputEvent::IE_Pressed, GameUIComp, &UCGameUIComponent::InputAction_ActivateEquipMenu);
	PlayerInputComponent->BindAction("Menu", EInputEvent::IE_Released, GameUIComp, &UCGameUIComponent::InputAction_DeactivateEquipMenu);
	PlayerInputComponent->BindAction("Action", EInputEvent::IE_Pressed, WeaponComp, &UCWeaponComponent::InputAction_Act);

	PlayerInputComponent->BindAction("Skill_1", EInputEvent::IE_Pressed, WeaponComp, &UCWeaponComponent::InputAction_Skill_1_Pressed);
	PlayerInputComponent->BindAction("Skill_1", EInputEvent::IE_Released, WeaponComp, &UCWeaponComponent::InputAction_Skill_1_Released);
	PlayerInputComponent->BindAction("Skill_2", EInputEvent::IE_Pressed, WeaponComp, &UCWeaponComponent::InputAction_Skill_2_Pressed);
	PlayerInputComponent->BindAction("Skill_2", EInputEvent::IE_Released, WeaponComp, &UCWeaponComponent::InputAction_Skill_2_Released);
	PlayerInputComponent->BindAction("Skill_3", EInputEvent::IE_Pressed, WeaponComp, &UCWeaponComponent::InputAction_Skill_3_Pressed);
	PlayerInputComponent->BindAction("Skill_3", EInputEvent::IE_Released, WeaponComp, &UCWeaponComponent::InputAction_Skill_3_Released);
}

void ACPlayableCharacter::InputAction_Avoid()
{
	CheckFalse(StateComp->IsIdleMode());
	CheckFalse(MovementComp->CanMove());

	StateComp->SetAvoidMode();
}

void ACPlayableCharacter::Avoid()
{
	CheckNull(MontagesComp);
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

	data.PlaySoundWave(this);
	data.PlayEffect(GetWorld(), GetActorLocation(), GetActorRotation());

	if (false == CharacterStatComp->IsDead())
	{
		const FVector start = GetActorLocation();

		CheckNull(Damage.Attacker);
		const FVector target = Damage.Attacker->GetActorLocation();

		FVector direction = target - start;
		direction.Normalize();

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

void ACPlayableCharacter::End_Hit()
{
	Super::End_Hit();

	switch (CurHitType)
	{
	case CEHitType::Knockback:
		StateComp->SetRiseMode();
		break;
	case CEHitType::Air:
	case CEHitType::Fly:
		StateComp->SetFallMode();
		break;
	default:
		StateComp->SetIdleMode();
		break;
	}

	StateComp->SetHitNoneMode();
}

void ACPlayableCharacter::End_Rise()
{
	StateComp->SetIdleMode();
}

void ACPlayableCharacter::OnStateTypeChanged(const CEStateType InPrevType, const CEStateType InNewType)
{
	switch (InNewType)
	{
	case CEStateType::Avoid:
		Avoid();
		break;
	case CEStateType::Rise:
		Rise();
		break;
	case CEStateType::Land:
		Land();
		break;
	case CEStateType::Dead:
		Dead();
		break;
	}
}

void ACPlayableCharacter::OnHitStateTypeChanged(const CEHitType InPrevType, const CEHitType InNewType)
{
	switch (InNewType)
	{
	case CEHitType::None:
		break;
	default:
		Hit();
		break;
	}
}

USpringArmComponent* ACPlayableCharacter::GetSpringArm() const
{
	return SpringArm;
}

UCTargetingComponent* ACPlayableCharacter::GetTargetingComp() const
{
	return TargetingComp;
}