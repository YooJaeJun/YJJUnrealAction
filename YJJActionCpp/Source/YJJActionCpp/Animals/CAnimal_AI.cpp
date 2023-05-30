#include "Animals/CAnimal_AI.h"
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
	CHelpers::CreateComponent<USceneComponent>(this, &RiderPoint, "RiderPoint", GetMesh());
	CHelpers::CreateComponent<USpringArmComponent>(this, &SpringArm, "SpringArm", RiderPoint);
	CHelpers::CreateComponent<UCameraComponent>(this, &Camera, "Camera", SpringArm);
	CHelpers::CreateComponent<UBoxComponent>(this, &InteractionCollision, "InteractionCollision", GetMesh());
	CHelpers::CreateComponent<USceneComponent>(this, &MountRight, "MountRight", GetMesh());
	CHelpers::CreateComponent<USceneComponent>(this, &MountLeft, "MountLeft", GetMesh());
	CHelpers::CreateComponent<USceneComponent>(this, &MountBack, "MountBack", GetMesh());
	CHelpers::CreateComponent<USceneComponent>(this, &Unmount, "Unmount", GetMesh());
	CHelpers::CreateComponent<USceneComponent>(this, &EyePoint, "EyePoint", GetMesh());
	CHelpers::CreateActorComponent<UCZoomComponent>(this, &ZoomComp, "ZoomComponent");
	CHelpers::CreateActorComponent<UCGameUIComponent>(this, &GameUIComp, "GameUIComponent");
	CHelpers::CreateActorComponent<UCPatrolComponent>(this, &PatrolComp, "PatrolComponent");
	CHelpers::CreateActorComponent<UCRidingComponent>(this, &RidingComp, "RidingComponent");
}

void ACAnimal_AI::BeginPlay()
{
	Super::BeginPlay();

	InteractionCollision->OnComponentBeginOverlap.AddDynamic(RidingComp, &UCRidingComponent::BeginOverlap);
	InteractionCollision->OnComponentEndOverlap.AddDynamic(RidingComp, &UCRidingComponent::EndOverlap);
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
