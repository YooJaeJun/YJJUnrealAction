#include "Characters/Animals/CAnimal_AI.h"
#include "Global.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CCamComponent.h"
#include "Components/CGameUIComponent.h"
#include "Components/CRidingComponent.h"
#include "Components/CPatrolComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/CTargetingComponent.h"
#include "Components/SceneComponent.h"
#include "Characters/AI/CAIController_Melee.h"
#include "Components/CMontagesComponent.h"

ACAnimal_AI::ACAnimal_AI()
{
	YJJHelpers::CreateComponent<USpringArmComponent>(this, &SpringArm, "SpringArm", GetMesh());
	YJJHelpers::CreateComponent<UCameraComponent>(this, &Camera, "Camera", SpringArm);
	YJJHelpers::CreateActorComponent<UCCamComponent>(this, &CamComp, "CamComponent");
	YJJHelpers::CreateActorComponent<UCGameUIComponent>(this, &GameUIComp, "GameUIComponent");
	YJJHelpers::CreateActorComponent<UCPatrolComponent>(this, &PatrolComp, "PatrolComponent");
	YJJHelpers::CreateActorComponent<UCRidingComponent>(this, &RidingComp, "RidingComponent");
	YJJHelpers::CreateActorComponent<UCWeaponComponent>(this, &WeaponComp, "WeaponComponent");
	YJJHelpers::CreateComponent<USceneComponent>(this, &MountLeftPoint, "MountLeftPoint", GetMesh());
	YJJHelpers::CreateComponent<USceneComponent>(this, &MountRightPoint, "MountRightPoint", GetMesh());
	YJJHelpers::CreateComponent<USceneComponent>(this, &MountBackPoint, "MountBackPoint", GetMesh());
	YJJHelpers::CreateComponent<USceneComponent>(this, &RiderPoint, "RiderPoint", GetMesh());
	YJJHelpers::CreateComponent<USceneComponent>(this, &UnmountPoint, "UnmountPoint", GetMesh());
	YJJHelpers::CreateComponent<UBoxComponent>(this, &InteractionCollision, "InteractionCollision", GetMesh());
	YJJHelpers::CreateComponent<USceneComponent>(this, &EyePoint, "EyePoint", GetMesh());

	if (IsValid(StateComp))
		StateComp->OnStateTypeChanged.AddUniqueDynamic(this, &ACAnimal_AI::OnStateTypeChanged);

	if (IsValid(MovementComp))
	{
		MovementComp->SetSpeeds(Speeds);
		MovementComp->SetSpeed(CESpeedType::Sprint);
		MovementComp->SetFriction(2.0f, 256.0f);
		MovementComp->SetJumpZ(700.0f);
	}

	if (IsValid(CamComp))
	{
		CamComp->EnableControlRotation();
		CamComp->DisableFixedCamera();
	}

	YJJHelpers::GetAsset<USoundBase>(&LandSound,
		TEXT("SoundCue'/Game/Assets/Sounds/Footsteps/Run/Stone/SC_Footstep_Stone_Run.SC_Footstep_Stone_Run'"));

	YJJHelpers::GetAsset<UFXSystemAsset>(&LandEffect,
		TEXT("NiagaraSystem'/Game/Assets/Effects/SuperheroFlight/VFX/Niagara/System/SuperheroLanding/NS_Superhero_Landing_Concrete.NS_Superhero_Landing_Concrete'"));

	YJJHelpers::GetClass<AActor>(&EyeClass, "Blueprint'/Game/Character/Animals/CBP_Eye.CBP_Eye_C'");

	if (IsValid(SpringArm))
	{
		SpringArm->bDoCollisionTest = false;
		SpringArm->SetRelativeLocation(FVector(0, 3, 100));
		SpringArm->SetRelativeRotation(FRotator(-5, 90, 0));
	}

	if (IsValid(MountLeftPoint))
		MountLeftPoint->SetRelativeLocation(FVector(40, 0, 80));
	if (IsValid(MountRightPoint))
		MountRightPoint->SetRelativeLocation(FVector(-40, 0, 80));
	if (IsValid(MountBackPoint))
		MountBackPoint->SetRelativeLocation(FVector(0, -60, 80));
	if (IsValid(UnmountPoint))
		UnmountPoint->SetRelativeLocation(FVector(-40, 0, 80));

	AIControllerClass = ACAIController_Melee::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ACAnimal_AI::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(RidingComp) && IsValid(InteractionCollision))
	{
		InteractionCollision->OnComponentBeginOverlap.AddDynamic(
			RidingComp, &UCRidingComponent::BeginOverlap);
		InteractionCollision->OnComponentEndOverlap.AddDynamic(
			RidingComp, &UCRidingComponent::EndOverlap);
	}

	if (IsValid(RiderPoint))
		RiderPoint->AttachToComponent(GetMesh(), 
			FAttachmentTransformRules::SnapToTargetIncludingScale, "Rider");

	if (IsValid(EyePoint))
	{
		EyePoint->AttachToComponent(GetMesh(), 
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, "EyeEffect");

		if (IsValid(EyeClass))
		{
			FActorSpawnParameters params;
			params.Owner = Cast<AActor>(this);

			Eye = GetWorld()->SpawnActor<AActor>(EyeClass, 
				EyePoint->GetComponentLocation(), EyePoint->GetComponentRotation(), params);

			const FAttachmentTransformRules attachRules(
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::KeepRelative,
				false);

			if (IsValid(Eye))
				Eye->AttachToComponent(GetMesh(), attachRules, "EyeEffect");
		}
	}

	if (IsValid(CharacterInfoComp))
		CharacterInfoComp->SetCharacterType(CECharacterType::Companion);

	if (IsValid(CharacterStatComp))
		CharacterStatComp->SetAttackRange(250.0f);
}

void ACAnimal_AI::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", MovementComp, &UCMovementComponent::InputAxis_MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", MovementComp, &UCMovementComponent::InputAxis_MoveRight);
	PlayerInputComponent->BindAxis("HorizontalLook", CamComp, &UCCamComponent::InputAxis_HorizontalLook);
	PlayerInputComponent->BindAxis("VerticalLook", CamComp, &UCCamComponent::InputAxis_VerticalLook);
	PlayerInputComponent->BindAxis("Zoom", CamComp, &UCCamComponent::InputAxis_Zoom);

	PlayerInputComponent->BindAction("Walk", IE_Pressed, MovementComp, &UCMovementComponent::InputAction_Walk);
	PlayerInputComponent->BindAction("Walk", IE_Released, MovementComp, &UCMovementComponent::InputAction_Run);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, MovementComp, &UCMovementComponent::InputAction_Jump);
	PlayerInputComponent->BindAction("Targeting", IE_Pressed, TargetingComp, &UCTargetingComponent::InputAction_Targeting);
	PlayerInputComponent->BindAction("Menu", IE_Pressed, GameUIComp, &UCGameUIComponent::InputAction_ActivateEquipMenu);
	PlayerInputComponent->BindAction("Menu", IE_Released, GameUIComp, &UCGameUIComponent::InputAction_DeactivateEquipMenu);
	PlayerInputComponent->BindAction("Action", IE_Pressed, WeaponComp, &UCWeaponComponent::InputAction_Act);
	PlayerInputComponent->BindAction("Action", IE_Pressed, RidingComp, &UCRidingComponent::InputAction_Act);
}

void ACAnimal_AI::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	UGameplayStatics::PlaySoundAtLocation(this, LandSound, GetActorLocation());

	FTransform landEffectTransform = GetActorTransform();
	landEffectTransform.SetScale3D(landEffectTransform.GetScale3D() * LandEffectScaleFactor);

	YJJHelpers::PlayEffect(GetWorld(), LandEffect, landEffectTransform);
}

void ACAnimal_AI::Hit()
{
	CheckNull(MontagesComp);
	MontagesComp->PlayAvoidAnim();

	Super::Hit();

	// µ¿¹°: HitDataÀÇ Montage, Launch ¾È ¾¸

	// Interaction
	const FHitData data = Damage.Event.HitData;

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

		SetActorRotation(UKismetMathLibrary::FindLookAtRotation(start, target));
	}

	if (CharacterStatComp->IsDead())
	{
		StateComp->SetDeadMode();
		return;
	}

	Damage.Attacker = nullptr;
	Damage.Causer = nullptr;
}

void ACAnimal_AI::OnStateTypeChanged(const CEStateType InPrevType, const CEStateType InNewType)
{
	switch (InNewType)
	{
	case CEStateType::Land:
		Land();
		break;
	case CEStateType::Dead:
		Dead();
		break;
	}
}

void ACAnimal_AI::SetZoomMinRange(const float InMinRange) const
{
	CheckNull(CamComp);
	CamComp->ZoomData.MinRange = InMinRange;
}

void ACAnimal_AI::SetZoomMaxRange(const float InMaxRange) const
{
	CheckNull(CamComp);
	CamComp->ZoomData.MaxRange = InMaxRange;
}

void ACAnimal_AI::OnHitStateTypeChanged(const CEHitType InPrevType, const CEHitType InNewType)
{
	Hit();
}

USpringArmComponent* ACAnimal_AI::GetSpringArm() const
{
	return SpringArm;
}

UCTargetingComponent* ACAnimal_AI::GetTargetingComp() const
{
	return TargetingComp;
}