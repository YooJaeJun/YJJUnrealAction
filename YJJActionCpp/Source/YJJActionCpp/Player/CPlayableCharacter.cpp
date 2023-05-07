#include "CPlayableCharacter.h"
#include "Global.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/CAnimInstance_Human.h"
#include "Components/CMovementComponent.h"
#include "Components/CMontagesComponent.h"
#include "Components/CWeaponComponent.h"
#include "Components/CZoomComponent.h"
#include "Components/CTargetingComponent.h"
#include "Components/CGameUIComponent.h"
#include "Game/CGameMode.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Widgets/Player/CUserWidget_PlayerHpBar.h"
#include "Widgets/Player/CUserWidget_PlayerInfo.h"

ACPlayableCharacter::ACPlayableCharacter()
	: ACCommonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CHelpers::CreateComponent<USpringArmComponent>(this, &SpringArm, "SpringArm", GetCapsuleComponent());
	CHelpers::CreateComponent<UCameraComponent>(this, &Camera, "Camera", SpringArm);
	CHelpers::CreateActorComponent<UCMontagesComponent>(this, &MontagesComponent, "MontagesComponent");
	CHelpers::CreateActorComponent<UCWeaponComponent>(this, &WeaponComponent, "WeaponComponent");
	CHelpers::CreateActorComponent<UCZoomComponent>(this, &ZoomComponent, "ZoomComponent");
	CHelpers::CreateActorComponent<UCTargetingComponent>(this, &TargetingComponent, "TargetingComponent");
	CHelpers::CreateActorComponent<UCGameUIComponent>(this, &GameUIComponent, "GameUIComponent");

	USkeletalMesh* mesh;
	CHelpers::GetAsset<USkeletalMesh>(&mesh, "SkeletalMesh'/Game/Assets/Character/MercenaryWarrior/Meshes/SK_MercenaryWarrior_WithoutHelmet.SK_MercenaryWarrior_WithoutHelmet'");

	GetMesh()->SetSkeletalMesh(mesh);
	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	TSubclassOf<UCAnimInstance_Human> animInstance;
	CHelpers::GetClass<UCAnimInstance_Human>(&animInstance, "AnimBlueprint'/Game/Character/ABP_CHuman.ABP_CHuman_C'");
	GetMesh()->SetAnimClass(animInstance);

	SpringArm->SetRelativeLocation(FVector(0, 0, 60));
	SpringArm->TargetArmLength = 280;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true;

	StateComponent->SetIdleMode();
	StateComponent->OnStateTypeChanged.AddDynamic(this, &ACPlayableCharacter::OnStateTypeChanged);

	MovementComponent->SetSpeeds(Speeds);
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->GroundFriction = 2;
	GetCharacterMovement()->BrakingDecelerationWalking = 256;

	CharacterStatComponent->OnHpIsZero.AddLambda([this]() -> void {
		MontagesComponent->PlayDeadAnim();
	});

	GameMode = Cast<ACGameMode>(UGameplayStatics::GetGameMode(AActor::GetWorld()));
}

void ACPlayableCharacter::BeginPlay()
{
	Super::BeginPlay();

	TWeakObjectPtr<APlayerController> playerController = Cast<APlayerController>(MyCurController);
	if (!!playerController.Get())
	{
		playerController->PlayerCameraManager->ViewPitchMin = PitchRange.X;
		playerController->PlayerCameraManager->ViewPitchMax = PitchRange.Y;
	}


	TWeakObjectPtr<UCUserWidget_HUD> hud = GameMode->GetHUD();
	if (hud.Get())
	{
		hud->SetChild();
		if (!!hud->PlayerInfo->HpBar)
			hud->PlayerInfo->HpBar->BindCharacterStat(CharacterStatComponent);
	}
}

void ACPlayableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACPlayableCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", MovementComponent, &UCMovementComponent::InputAxis_MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", MovementComponent, &UCMovementComponent::InputAxis_MoveRight);
	PlayerInputComponent->BindAxis("HorizontalLook", MovementComponent, &UCMovementComponent::InputAxis_HorizontalLook);
	PlayerInputComponent->BindAxis("VerticalLook", MovementComponent, &UCMovementComponent::InputAxis_VerticalLook);
	PlayerInputComponent->BindAxis("Zoom", ZoomComponent, &UCZoomComponent::InputAxis_Zoom);

	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Pressed, MovementComponent, &UCMovementComponent::InputAction_Walk);
	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Released, MovementComponent, &UCMovementComponent::InputAction_Run);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, MovementComponent, &UCMovementComponent::InputAction_Jump);
	PlayerInputComponent->BindAction("Avoid", EInputEvent::IE_Pressed, this, &ACPlayableCharacter::InputAction_Avoid);
	PlayerInputComponent->BindAction("Targeting", EInputEvent::IE_Pressed, TargetingComponent, &UCTargetingComponent::InputAction_Targeting);
	PlayerInputComponent->BindAction("Menu", EInputEvent::IE_Pressed, GameUIComponent, &UCGameUIComponent::InputAction_ActivateEquipMenu);
	PlayerInputComponent->BindAction("Menu", EInputEvent::IE_Released, GameUIComponent, &UCGameUIComponent::InputAction_DeactivateEquipMenu);
}

void ACPlayableCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	StateComponent->SetIdleMode();
}

void ACPlayableCharacter::Avoid()
{
	MovementComponent->EnableControlRotation();

	MontagesComponent->PlayAvoidAnim();
}

void ACPlayableCharacter::End_Avoid()
{
	MovementComponent->DisableControlRotation();

	StateComponent->SetIdleMode();
}

void ACPlayableCharacter::InputAction_Avoid()
{
	CheckFalse(StateComponent->IsIdleMode());
	CheckFalse(MovementComponent->CanMove());

	if (InputComponent->GetAxisValue("MoveForward") >= 0.0f)
		return;

	StateComponent->SetAvoidMode();
}

void ACPlayableCharacter::OnStateTypeChanged(const EStateType InPrevType, const EStateType InNewType)
{
	switch (static_cast<uint8>(InNewType))
	{
	case EStateType::Avoid:
		Avoid();
		break;
	}
}
