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
#include "Widgets/Player/CUserWidget_PlayerBar.h"
#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "Widgets/Player/CUserWidget_PlayerLevel.h"

ACPlayableCharacter::ACPlayableCharacter()
	: ACCommonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CHelpers::CreateComponent<USpringArmComponent>(this, &SpringArm, "SpringArm", GetCapsuleComponent());
	CHelpers::CreateComponent<UCameraComponent>(this, &Camera, "Camera", SpringArm);
	CHelpers::CreateActorComponent<UCMontagesComponent>(this, &MontagesComp, "MontagesComponent");
	CHelpers::CreateActorComponent<UCWeaponComponent>(this, &WeaponComp, "WeaponComponent");
	CHelpers::CreateActorComponent<UCZoomComponent>(this, &ZoomComp, "ZoomComponent");
	CHelpers::CreateActorComponent<UCTargetingComponent>(this, &TargetingComp, "TargetingComponent");
	CHelpers::CreateActorComponent<UCGameUIComponent>(this, &GameUIComp, "GameUIComponent");

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

	StateComp->SetIdleMode();
	StateComp->OnStateTypeChanged.AddDynamic(this, &ACPlayableCharacter::OnStateTypeChanged);

	MovementComp->SetSpeeds(Speeds);
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->GroundFriction = 2;
	GetCharacterMovement()->BrakingDecelerationWalking = 256;

	CharacterStatComp->OnHpIsZero.AddLambda([this]() -> void {
		MontagesComp->PlayDeadAnim();
		GetWorldTimerManager().SetTimer(DestroyDelayTimerHandle, [this]() -> void {
			Destroy();
		}, 1.5f, false, 1.5f);
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

		TWeakObjectPtr<UCUserWidget_PlayerInfo> playerInfo = hud->PlayerInfo;
		if (!!playerInfo.Get())
		{
			if (!!playerInfo->LevelBar)
				hud->PlayerInfo->LevelBar->BindLevelStat(CharacterStatComp);

			if (!!playerInfo->HpBar)
				hud->PlayerInfo->HpBar->BindHpStat(CharacterStatComp);

			if (!!playerInfo->StaminaBar)
				hud->PlayerInfo->StaminaBar->BindStaminaStat(CharacterStatComp);

			if (!!playerInfo->ManaBar)
				hud->PlayerInfo->ManaBar->BindManaStat(CharacterStatComp);
		}
	}
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
	PlayerInputComponent->BindAction("Action", EInputEvent::IE_Pressed, WeaponComp, &UCWeaponComponent::Act);
}

void ACPlayableCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	StateComp->SetIdleMode();
}

void ACPlayableCharacter::Avoid()
{
	MovementComp->EnableControlRotation();

	MontagesComp->PlayAvoidAnim();
}

void ACPlayableCharacter::End_Avoid()
{
	MovementComp->DisableControlRotation();

	StateComp->SetIdleMode();
}

void ACPlayableCharacter::End_Hit()
{
	StateComp->SetIdleMode();
}

void ACPlayableCharacter::InputAction_Avoid()
{
	CheckFalse(StateComp->IsIdleMode());
	CheckFalse(MovementComp->CanMove());

	StateComp->SetAvoidMode();
}

void ACPlayableCharacter::OnStateTypeChanged(const EStateType InPrevType, const EStateType InNewType)
{
	switch (static_cast<uint8>(InNewType))
	{
	case static_cast<uint8>(EStateType::Avoid):
		Avoid();
		break;
	}
}
