#include "Characters/Animals/CAnimal_AI.h"
#include "Global.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CZoomComponent.h"
#include "Components/CGameUIComponent.h"
#include "Components/CRidingComponent.h"
#include "Components/CPatrolComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"

ACAnimal_AI::ACAnimal_AI()
{
	CHelpers::CreateComponent<USpringArmComponent>(this, &SpringArm, "SpringArm", GetMesh());
	CHelpers::CreateComponent<UCameraComponent>(this, &Camera, "Camera", SpringArm);
	CHelpers::CreateActorComponent<UCZoomComponent>(this, &ZoomComp, "ZoomComponent");
	CHelpers::CreateActorComponent<UCGameUIComponent>(this, &GameUIComp, "GameUIComponent");
	CHelpers::CreateActorComponent<UCPatrolComponent>(this, &PatrolComp, "PatrolComponent");
	CHelpers::CreateActorComponent<UCRidingComponent>(this, &RidingComp, "RidingComponent");
	CHelpers::CreateComponent<USceneComponent>(this, &MountLeftPoint, "MountLeftPoint", GetMesh());
	CHelpers::CreateComponent<USceneComponent>(this, &MountRightPoint, "MountRightPoint", GetMesh());
	CHelpers::CreateComponent<USceneComponent>(this, &MountBackPoint, "MountBackPoint", GetMesh());
	CHelpers::CreateComponent<USceneComponent>(this, &RiderPoint, "RiderPoint", GetMesh());
	CHelpers::CreateComponent<USceneComponent>(this, &UnmountPoint, "UnmountPoint", GetMesh());
	CHelpers::CreateComponent<USceneComponent>(this, &EyePoint, "EyePoint", GetMesh());
	CHelpers::CreateComponent<UBoxComponent>(this, &InteractionCollision, "InteractionCollision", GetMesh());

	if (!!MovementComp)
	{
		MovementComp->SetSpeeds(Speeds);
		MovementComp->EnableControlRotation();
		MovementComp->UnFixCamera();
		MovementComp->SetSpeed(ESpeedType::Sprint);
		MovementComp->SetFriction(2, 256);
		MovementComp->SetJumpZ(700.0f);
	}

	if (!!RiderPoint)
		RiderPoint->SetWorldLocation(FindComponentByClass<USceneComponent>()->GetSocketLocation("RiderPoint"));

	CHelpers::LoadAsset<USoundBase>(&LandSound,
		TEXT("SoundCue'/Game/Assets/Sounds/Footsteps/Run/Stone/SC_Footstep_Stone_Run.SC_Footstep_Stone_Run'"));

	CHelpers::LoadAsset<UFXSystemAsset>(&LandEffect,
		TEXT("NiagaraSystem'/Game/Assets/Effects/SuperheroFlight/VFX/Niagara/System/SuperheroLanding/NS_Superhero_Landing_Concrete.NS_Superhero_Landing_Concrete'"));
}

void ACAnimal_AI::BeginPlay()
{
	Super::BeginPlay();

	if (!!InteractionCollision)
	{
		InteractionCollision->OnComponentBeginOverlap.AddDynamic(RidingComp, &UCRidingComponent::BeginOverlap);
		InteractionCollision->OnComponentEndOverlap.AddDynamic(RidingComp, &UCRidingComponent::EndOverlap);
	}
}

void ACAnimal_AI::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
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
	PlayerInputComponent->BindAction("Menu", EInputEvent::IE_Pressed, GameUIComp, &UCGameUIComponent::InputAction_ActivateEquipMenu);
	PlayerInputComponent->BindAction("Menu", EInputEvent::IE_Released, GameUIComp, &UCGameUIComponent::InputAction_DeactivateEquipMenu);
}

void ACAnimal_AI::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	UGameplayStatics::PlaySoundAtLocation(this, LandSound, GetActorLocation());
	CHelpers::PlayEffect(GetWorld(), LandEffect, GetActorTransform());
}