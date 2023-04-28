#include "CPlayableCharacter.h"
#include "Global.h"
#include "Character/CAnimInstance_Human.h"
#include "Component/CWeaponComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Component/CMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ACPlayableCharacter::ACPlayableCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CHelpers::CreateComponent<USpringArmComponent>(this, &SpringArm, "SpringArm", GetCapsuleComponent());
	CHelpers::CreateComponent<UCameraComponent>(this, &Camera, "Camera", SpringArm);
	CHelpers::CreateActorComponent<UCWeaponComponent>(this, &WeaponComponent, "WeaponComponent");

	USkeletalMesh* mesh;
	CHelpers::GetAsset<USkeletalMesh>(&mesh, "SkeletalMesh'/Game/Assets/Character/MercenaryWarrior/Meshes/SK_MercenaryWarrior_WithoutHelmet.SK_MercenaryWarrior_WithoutHelmet'");

	GetMesh()->SetSkeletalMesh(mesh);
	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	TSubclassOf<UCAnimInstance_Human> animInstance;
	CHelpers::GetClass<UCAnimInstance_Human>(&animInstance, "AnimBlueprint'/Game/Character/ABP_CHuman.ABP_CHuman_C'");
	GetMesh()->SetAnimClass(animInstance);

	SpringArm->SetRelativeLocation(FVector(0, 0, 60));
	SpringArm->TargetArmLength = 200;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true;

	MovementComponent->SetSpeeds(Speeds);
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->GroundFriction = 2;
	GetCharacterMovement()->BrakingDecelerationWalking = 256;
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

	PlayerInputComponent->BindAxis("MoveForward", this, &ACPlayableCharacter::OnMoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACPlayableCharacter::OnMoveRight);
	PlayerInputComponent->BindAxis("HorizontalLook", this, &ACPlayableCharacter::OnHorizontalLook);
	PlayerInputComponent->BindAxis("VerticalLook", this, &ACPlayableCharacter::OnVerticalLook);

	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Pressed, this, &ACPlayableCharacter::SetWalkMode);
	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Released, this, &ACPlayableCharacter::SetRunMode);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACPlayableCharacter::SetJumpMode);
}

void ACPlayableCharacter::OnMoveForward(const float InAxisValue)
{
	const FRotator rotator = FRotator(0, GetControlRotation().Yaw, 0);
	const FVector direction = FQuat(rotator).GetForwardVector();

	AddMovementInput(direction, InAxisValue);
}

void ACPlayableCharacter::OnMoveRight(const float InAxisValue)
{
	const FRotator rotator = FRotator(0, GetControlRotation().Yaw, 0);
	const FVector direction = FQuat(rotator).GetRightVector();

	AddMovementInput(direction, InAxisValue);
}

void ACPlayableCharacter::OnHorizontalLook(const float InAxisValue)
{
	AddControllerYawInput(InAxisValue);
}

void ACPlayableCharacter::OnVerticalLook(const float InAxisValue)
{
	AddControllerPitchInput(InAxisValue);
}

void ACPlayableCharacter::SetJumpMode()
{
	CheckFalse(MovementComponent->CanMove());

	Jump();

	StateComponent->SetFallingMode();
}

void ACPlayableCharacter::SetWalkMode()
{
	MovementComponent->SetMaxWalkSpeed(Speeds[static_cast<uint8>(ESpeedType::Walk)]);
}

void ACPlayableCharacter::SetRunMode()
{
	MovementComponent->SetMaxWalkSpeed(Speeds[static_cast<uint8>(ESpeedType::Sprint)]);
}