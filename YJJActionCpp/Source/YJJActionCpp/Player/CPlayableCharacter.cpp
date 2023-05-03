#include "CPlayableCharacter.h"
#include "Global.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/CameraComponent.h"
#include "Character/CAnimInstance_Human.h"
#include "Component/CMovementComponent.h"
#include "Component/CMontagesComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CZoomComponent.h"
#include "Component/CTargetingComponent.h"
#include "Component/CGameUIComponent.h"
#include "GameMode/CGameMode.h"

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

	MovementComponent->SetSpeeds(Speeds);
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->GroundFriction = 2;
	GetCharacterMovement()->BrakingDecelerationWalking = 256;

	StateComponent->SetIdleMode();
	StateComponent->OnStateTypeChanged.AddDynamic(this, &ACPlayableCharacter::OnStateTypeChanged);

	Name = TEXT("플레이어");
}

void ACPlayableCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetController<APlayerController>()->PlayerCameraManager->ViewPitchMin = PitchRange.X;
	GetController<APlayerController>()->PlayerCameraManager->ViewPitchMax = PitchRange.Y;
}

void ACPlayableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACPlayableCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", MovementComponent, &UCMovementComponent::OnMoveForward);
	PlayerInputComponent->BindAxis("MoveRight", MovementComponent, &UCMovementComponent::OnMoveRight);
	PlayerInputComponent->BindAxis("HorizontalLook", MovementComponent, &UCMovementComponent::OnHorizontalLook);
	PlayerInputComponent->BindAxis("VerticalLook", MovementComponent, &UCMovementComponent::OnVerticalLook);
	PlayerInputComponent->BindAxis("Zoom", ZoomComponent, &UCZoomComponent::OnZoom);

	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Pressed, MovementComponent, &UCMovementComponent::OnWalk);
	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Released, MovementComponent, &UCMovementComponent::OnRun);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, MovementComponent, &UCMovementComponent::OnJump);
	PlayerInputComponent->BindAction("Avoid", EInputEvent::IE_Pressed, this, &ACPlayableCharacter::OnAvoid);
	PlayerInputComponent->BindAction("Targeting", EInputEvent::IE_Pressed, TargetingComponent, &UCTargetingComponent::OnTargeting);
	PlayerInputComponent->BindAction("Menu", EInputEvent::IE_Pressed, GameUIComponent, &UCGameUIComponent::OnMenu);
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

void ACPlayableCharacter::OnAvoid()
{
	CheckFalse(StateComponent->IsIdleMode());
	CheckFalse(MovementComponent->CanMove());

	if (InputComponent->GetAxisValue("MoveForward") >= 0.0f)
		return;

	StateComponent->SetAvoidMode();
}

void ACPlayableCharacter::OnStateTypeChanged(const EStateType InPrevType, const EStateType InNewType)
{
	switch (InNewType)
	{
	case EStateType::Avoid:
		Avoid();
		break;
	}
}
