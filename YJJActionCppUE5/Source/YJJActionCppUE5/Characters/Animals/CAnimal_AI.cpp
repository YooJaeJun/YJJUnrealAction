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
#include "Components/WidgetComponent.h"
#include "Characters/AI/CAIController_Melee.h"
#include "Components/CMontagesComponent.h"
#include "Blueprint/UserWidget.h"

ACAnimal_AI::ACAnimal_AI()
{
	const TObjectPtr<USkeletalMeshComponent> mesh = GetMesh();

	// mesh ?? ??�???? ???(BP_Animal ? ?? ??).
	YJJHelpers::CreateComponent<USceneComponent>(this, &MountLeftPoint, TEXT("MountLeft"), mesh);
	YJJHelpers::CreateComponent<USceneComponent>(this, &MountRightPoint, TEXT("MountRight"), mesh);
	YJJHelpers::CreateComponent<USceneComponent>(this, &MountBackPoint, TEXT("MountBack"), mesh);
	YJJHelpers::CreateComponent<USceneComponent>(this, &RiderPoint, TEXT("RiderPoint"), mesh);
	YJJHelpers::CreateComponent<USceneComponent>(this, &UnmountPoint, TEXT("Unmount"), mesh);
	YJJHelpers::CreateComponent<UBoxComponent>(this, &InteractionCollision, TEXT("InterationCollision"), mesh);
	YJJHelpers::CreateComponent<USceneComponent>(this, &EyePoint, TEXT("EyePoint"), mesh);

	YJJHelpers::CreateComponent<USpringArmComponent>(this, &SpringArm, TEXT("SpringArm"), RiderPoint);
	YJJHelpers::CreateComponent<UCameraComponent>(this, &Camera, TEXT("Camera"), SpringArm);

	YJJHelpers::CreateComponent<USceneComponent>(this, &HpBarSceneRoot, TEXT("Scene"), GetCapsuleComponent());
	YJJHelpers::CreateComponent<UWidgetComponent>(this, &HpBarWidgetComp, TEXT("HpBarWidget"), HpBarSceneRoot);

	YJJHelpers::CreateActorComponent<UCCamComponent>(this, &CamComp, "CamComponent");
	YJJHelpers::CreateActorComponent<UCGameUIComponent>(this, &GameUIComp, "GameUIComponent");
	YJJHelpers::CreateActorComponent<UCPatrolComponent>(this, &PatrolComp, "PatrolComponent");
	YJJHelpers::CreateActorComponent<UCRidingComponent>(this, &RidingComp, "RidingComponent");
	RidingComponent = RidingComp;
	YJJHelpers::CreateActorComponent<UCWeaponComponent>(this, &WeaponComp, "WeaponComponent");
	YJJHelpers::CreateActorComponent<UCTargetingComponent>(this, &TargetingComp, "TargetingComponent");

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
		TEXT("/Script/Engine.SoundCue'/Game/Assets/Sounds/Footsteps/Run/Stone/SC_Footstep_Stone_Run.SC_Footstep_Stone_Run'"));

	YJJHelpers::GetAsset<UFXSystemAsset>(&LandEffect,
		TEXT("/Script/Niagara.NiagaraSystem'/Game/Assets/Effects/SuperheroFlight/VFX/Niagara/System/SuperheroLanding/NS_Superhero_Landing_Concrete.NS_Superhero_Landing_Concrete'"));

	YJJHelpers::GetClass<AActor>(&EyeClass, "/Script/Engine.Blueprint'/Game/Character/Animals/CBP_Eye.CBP_Eye_C'");

	if (IsValid(SpringArm))
	{
		SpringArm->bDoCollisionTest = false;
		SpringArm->SetRelativeLocation(FVector(0, 3, 100));
		SpringArm->SetRelativeRotation(FRotator(-5, 90, 0));
	}

	if (IsValid(HpBarWidgetComp))
	{
		HpBarWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
		HpBarWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HpBarWidgetComp->SetDrawAtDesiredSize(true);
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

	if (IsValid(HpBarWidgetComp) && AnimalHpBarWidgetClass != nullptr)
		HpBarWidgetComp->SetWidgetClass(AnimalHpBarWidgetClass);

	if (IsValid(RidingComp) && IsValid(InteractionCollision))
	{
		InteractionCollision->OnComponentBeginOverlap.AddDynamic(
			RidingComp.Get(), &UCRidingComponent::MountInteraction_OnBeginOverlap);
		InteractionCollision->OnComponentEndOverlap.AddDynamic(
			RidingComp.Get(), &UCRidingComponent::MountInteraction_OnEndOverlap);
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

	// HpBarWidgetComp ? ?? ???? ?? ? SetHpUI ? HpBar_NPC ?? ? ??? ???.
	SetHpUI();
}

UWidgetComponent* ACAnimal_AI::GetAnimalHpBarWidgetComponent() const
{
	return HpBarWidgetComp.Get();
}

void ACAnimal_AI::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", MovementComp.Get(), &UCMovementComponent::InputAxis_MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", MovementComp.Get(), &UCMovementComponent::InputAxis_MoveRight);
	PlayerInputComponent->BindAxis("HorizontalLook", CamComp.Get(), &UCCamComponent::InputAxis_HorizontalLook);
	PlayerInputComponent->BindAxis("VerticalLook", CamComp.Get(), &UCCamComponent::InputAxis_VerticalLook);
	PlayerInputComponent->BindAxis("Zoom", RidingComp.Get(), &UCRidingComponent::Input_Zoom);

	PlayerInputComponent->BindAction("Walk", IE_Pressed, MovementComp.Get(), &UCMovementComponent::InputAction_Walk);
	PlayerInputComponent->BindAction("Walk", IE_Released, MovementComp.Get(), &UCMovementComponent::InputAction_Run);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, MovementComp.Get(), &UCMovementComponent::InputAction_Jump);
	PlayerInputComponent->BindAction("Targeting", IE_Pressed, RidingComp.Get(), &UCRidingComponent::TargetingInput);
	PlayerInputComponent->BindAction("Menu", IE_Pressed, RidingComp.Get(), &UCRidingComponent::Ride_Input_Menu);
	PlayerInputComponent->BindAction("Menu", IE_Released, RidingComp.Get(), &UCRidingComponent::Ride_Input_MenuHide);
	PlayerInputComponent->BindAction("MagicMenu", IE_Pressed, RidingComp.Get(), &UCRidingComponent::Ride_Input_MagicMenu);
	PlayerInputComponent->BindAction("MagicMenu", IE_Released, RidingComp.Get(), &UCRidingComponent::Ride_Input_MagicMenuHide);
	PlayerInputComponent->BindAction("Action", IE_Pressed, RidingComp.Get(), &UCRidingComponent::Ride_Input_Action);
	PlayerInputComponent->BindAction("SubWeapon_Action", IE_Pressed, RidingComp.Get(), &UCRidingComponent::Ride_Input_SubWeaponPressed);
	PlayerInputComponent->BindAction("SubWeapon_Action", IE_Released, RidingComp.Get(), &UCRidingComponent::Ride_Input_SubWeaponReleased);
	PlayerInputComponent->BindAction("Skill_1", IE_Pressed, RidingComp.Get(), &UCRidingComponent::Ride_Input_Skill1Pressed);
	PlayerInputComponent->BindAction("Skill_2", IE_Pressed, RidingComp.Get(), &UCRidingComponent::Ride_Input_Skill2Pressed);
	PlayerInputComponent->BindAction("Magic", IE_Pressed, RidingComp.Get(), &UCRidingComponent::Ride_Input_Magic);
}

void ACAnimal_AI::InputAction_Interact()
{
	if (GetbRiding() && IsValid(RidingComp.Get()) && RidingComp->GetRider().IsValid())
	{
		RidingComp->EndInteraction();
		return;
	}

	ACCommonCharacter::InputAction_Interact();
}

void ACAnimal_AI::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// BP_OnLanded ?? ?? ?? ? � ??/?? ??? ?? ???? ????.
	if (IsValid(MovementComp))
		MovementComp->SetGravity(1.f);

	PlayLandMontageIfAny();

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

	// ?? HitData ??? Launch ??? ??? ??? ??.

	// HitData ?? ?? ??�???�???
	const FHitData data = Damage.Event.HitData;

	data.PlayHitStop(GetWorld());
	data.PlaySoundWave(this);
	data.PlayEffect(GetWorld(), Damage.Event.HitData.EffectLocation, GetActorRotation());

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

TObjectPtr<USpringArmComponent> ACAnimal_AI::GetSpringArm() const
{
	return SpringArm;
}

TObjectPtr<UCTargetingComponent> ACAnimal_AI::GetTargetingComp() const
{
	return TargetingComp;
}