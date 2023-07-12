#include "Characters/Animals/Dragon/CDragon_AI.h"
#include "Global.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CTargetingComponent.h"
#include "Components/CGameUIComponent.h"
#include "Components/CRidingComponent.h"
#include "Components/CFlyComponent.h"

ACDragon_AI::ACDragon_AI()
{
	YJJHelpers::CreateActorComponent<UCFlyComponent>(this, &FlyComp, "FlyComponent");
}

void ACDragon_AI::BeginPlay()
{
	Super::BeginPlay();

	SetZoomMinRange(50.0f);
	SetZoomMaxRange(1000.0f);

	LandEffectScaleFactor = 2.0f;

	if (IsValid(CharacterStatComp))
		CharacterStatComp->SetAttackRange(1200.0f);
}

void ACDragon_AI::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", FlyComp, &UCFlyComponent::InputAxis_MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", FlyComp, &UCFlyComponent::InputAxis_MoveRight);
	PlayerInputComponent->BindAxis("HorizontalLook", FlyComp, &UCFlyComponent::InputAxis_HorizontalLook);
	PlayerInputComponent->BindAxis("VerticalLook", FlyComp, &UCFlyComponent::InputAxis_VerticalLook);
	PlayerInputComponent->BindAxis("FlyUp", FlyComp, &UCFlyComponent::InputAxis_FlyUp);
	PlayerInputComponent->BindAxis("Zoom", CamComp, &UCCamComponent::InputAxis_Zoom);

	PlayerInputComponent->BindAction("Walk", IE_Pressed, MovementComp, &UCMovementComponent::InputAction_Walk);
	PlayerInputComponent->BindAction("Walk", IE_Released, MovementComp, &UCMovementComponent::InputAction_Run);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, FlyComp, &UCFlyComponent::InputAction_Jump);
	PlayerInputComponent->BindAction("Targeting", IE_Pressed, TargetingComp, &UCTargetingComponent::InputAction_Targeting);
	PlayerInputComponent->BindAction("Menu", IE_Pressed, GameUIComp, &UCGameUIComponent::InputAction_ActivateEquipMenu);
	PlayerInputComponent->BindAction("Menu", IE_Released, GameUIComp, &UCGameUIComponent::InputAction_DeactivateEquipMenu);
	PlayerInputComponent->BindAction("Action", IE_Pressed, RidingComp, &UCRidingComponent::InputAction_Act);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ACCommonCharacter::InputAction_Interact);
}

void ACDragon_AI::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	CheckNull(FlyComp);

	FlyComp->LandOn();
}