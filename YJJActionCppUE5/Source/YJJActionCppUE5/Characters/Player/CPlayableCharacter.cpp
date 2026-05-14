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
#include "Components/CInventoryComponent.h"
#include "Components/CPlacementComponent.h"
#include "Commons/CGameMode.h"
#include "Commons/CPlayerController.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/CCharacterStatComponent.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "Widgets/Weapons/CUserWidget_EquipMenu.h"
#include "Widgets/Weapons/CUserWidget_MagicMenu.h"
#include "Widgets/Interaction/CUserWidget_Interaction.h"
#include "Components/CRidingComponent.h"

ACPlayableCharacter::ACPlayableCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	const TObjectPtr<USkeletalMeshComponent> meshComp = GetMesh();

	YJJHelpers::CreateComponent<USpringArmComponent>(this, &SpringArm, "SpringArm", meshComp);
	YJJHelpers::CreateComponent<UCameraComponent>(this, &Camera, "Camera", SpringArm);
	YJJHelpers::CreateActorComponent<UCWeaponComponent>(this, &WeaponComp, "WeaponComponent");
	YJJHelpers::CreateActorComponent<UCCamComponent>(this, &CamComp, "CamComponent");
	YJJHelpers::CreateActorComponent<UCTargetingComponent>(this, &TargetingComp, "TargetingComponent");
	YJJHelpers::CreateActorComponent<UCGameUIComponent>(this, &GameUIComp, "GameUIComponent");
	YJJHelpers::CreateActorComponent<UCInventoryComponent>(this, &InventoryComp, "InventoryComponent");
	YJJHelpers::CreateActorComponent<UCPlacementComponent>(this, &PlacementComp, "PlacementComponent");

	TObjectPtr<USkeletalMesh> mesh = nullptr;
	YJJHelpers::GetAsset<USkeletalMesh>(&mesh, "SkeletalMesh'/Game/Assets/Character/MercenaryWarrior/Meshes/SK_MercenaryWarrior_WithoutHelmet.SK_MercenaryWarrior_WithoutHelmet'");

	GetMesh()->SetSkeletalMeshAsset(mesh.Get());
	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	TSubclassOf<UCAnimInstance_Human> animInstance;
	YJJHelpers::GetClass<UCAnimInstance_Human>(&animInstance, "AnimBlueprint'/Game/Character/CABP_Human.CABP_Human_C'");
	GetMesh()->SetAnimInstanceClass(animInstance);

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

	if (IsValid(MovementComp))
		MovementComp->SetSpeed(CESpeedType::Sprint);

	const TWeakObjectPtr<APlayerController> playerController = Cast<APlayerController>(GetController());
	if (playerController.IsValid())
	{
		playerController->PlayerCameraManager->ViewPitchMin = PitchRange.X;
		playerController->PlayerCameraManager->ViewPitchMax = PitchRange.Y;
	}

	TObjectPtr<ACPlayerController> yjjPlayerController = Cast<ACPlayerController>(GetController());
	if (IsLocallyControlled())
	{
		if (IsValid(yjjPlayerController))
			yjjPlayerController->InitializeHUDForPawn(this);
		else
			CLog::Log(FString::Printf(TEXT("[UI] CPlayableCharacter::BeginPlay: 로컬 조종인데 ACPlayerController 가 아님 또는 무효 — %s"), *GetName()));
	}

	if (IsValid(CharacterInfoComp))
		CharacterInfoComp->SetCharacterType(CECharacterType::Player);
}

void ACPlayableCharacter::SetStatusUI()
{
	if (false == IsLocallyControlled())
		return;

	UCUserWidget_HUD* hud = GetPlayerHUDWidget();
	if (false == IsValid(hud))
	{
		CLog::Log(TEXT("[UI] SetStatusUI: HUD 없음"));
		return;
	}

	UCUserWidget_PlayerInfo* playerInfo = hud->GetPlayerInfoWidget();
	if (false == IsValid(playerInfo))
	{
		CLog::Log(TEXT("[UI] SetStatusUI: PlayerInfo 없음"));
		return;
	}

	if (false == IsValid(CharacterStatComp))
	{
		CLog::Log(TEXT("[UI] SetStatusUI: CharacterStatComp 없음"));
		return;
	}

	playerInfo->BindStats(CharacterStatComp);
	playerInfo->RefreshPlayerInfoWidgets();

	hud->SetVisibility(ESlateVisibility::Visible);
}

void ACPlayableCharacter::SetMenuUI()
{
	if (false == IsLocallyControlled())
		return;

	APlayerController* genericPc = Cast<APlayerController>(GetController());
	if (false == IsValid(genericPc))
	{
		CLog::Log(FString::Printf(TEXT("[UI] SetMenuUI: PlayerController 무효 — %s"), *GetName()));
		return;
	}

	ACPlayerController* yjjPc = Cast<ACPlayerController>(genericPc);
	UCUserWidget_HUD* hud = IsValid(yjjPc) ? yjjPc->EnsureHUD() : nullptr;
	if (false == IsValid(hud))
	{
		CLog::Log(FString::Printf(TEXT("[UI] SetMenuUI: HUD 없음 — %s"), *GetName()));
		return;
	}

	hud->SetChildren();

	UCUserWidget_EquipMenu* equipMenu = hud->GetEquipMenuWidget();
	if (IsValid(equipMenu))
	{
		MenuEquipWidget = equipMenu;
		equipMenu->SetVisibility(ESlateVisibility::Hidden);
		// SetMenuUI 가 여러 번 호출돼도 동일 핸들러가 중복되지 않게 먼저 제거한다.
		equipMenu->OnWeaponEquipped.RemoveDynamic(this, &ACPlayableCharacter::EquipWeaponFromUI);
		equipMenu->OnWeaponEquipped.AddDynamic(this, &ACPlayableCharacter::EquipWeaponFromUI);
	}
	else
		CLog::Log(FString::Printf(TEXT("[UI] SetMenuUI: EquipMenu(CEquipMenu) 없음 — %s"), *GetName()));

	UCUserWidget_MagicMenu* magicMenu = hud->GetMagicMenuWidget();
	if (IsValid(magicMenu))
	{
		MenuMagicWidget = magicMenu;
		magicMenu->SetVisibility(ESlateVisibility::Hidden);
		magicMenu->OnEquipMagic.RemoveDynamic(this, &ACPlayableCharacter::EquipMagicFromUI);
		magicMenu->OnEquipMagic.AddDynamic(this, &ACPlayableCharacter::EquipMagicFromUI);
	}
	else
		CLog::Log(FString::Printf(TEXT("[UI] SetMenuUI: MagicMenu(CMagicMenu) 없음 — HUD에 WB_MagicMenu 를 넣고 Parent=UCUserWidget_MagicMenu, 이름 CMagicMenu — %s"), *GetName()));

	UCUserWidget_Interaction* interactionWidget = hud->GetInteractionWidget();
	if (IsValid(interactionWidget))
		MenuInteractionWidget = interactionWidget;
	else
		CLog::Log(FString::Printf(TEXT("[UI] SetMenuUI: Interaction(CInteraction) 없음 — %s"), *GetName()));
}

void ACPlayableCharacter::EquipWeaponFromUI(const CEWeaponType InNewType)
{
	if (false == IsValid(WeaponComp))
	{
		CLog::Log(TEXT("[UI] EquipWeaponFromUI: WeaponComp 없음"));
		return;
	}

	WeaponComp->SetMode(InNewType);
}

void ACPlayableCharacter::EquipMagicFromUI(const CEWeaponType InNewType)
{
	// 마법·무기는 동일 UCWeaponComponent 모드 전환으로 처리한다.
	EquipWeaponFromUI(InNewType);
}

void ACPlayableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACPlayableCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", MovementComp.Get(), &UCMovementComponent::InputAxis_MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", MovementComp.Get(), &UCMovementComponent::InputAxis_MoveRight);
	PlayerInputComponent->BindAxis("HorizontalLook", CamComp.Get(), &UCCamComponent::InputAxis_HorizontalLook);
	PlayerInputComponent->BindAxis("VerticalLook", CamComp.Get(), &UCCamComponent::InputAxis_VerticalLook);
	PlayerInputComponent->BindAxis("Zoom", CamComp.Get(), &UCCamComponent::InputAxis_Zoom);

	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Pressed, MovementComp.Get(), &UCMovementComponent::InputAction_Walk);
	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Released, MovementComp.Get(), &UCMovementComponent::InputAction_Run);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, MovementComp.Get(), &UCMovementComponent::InputAction_Jump);
	PlayerInputComponent->BindAction("Avoid", EInputEvent::IE_Pressed, this, &ACPlayableCharacter::InputAction_Avoid);
	PlayerInputComponent->BindAction("Targeting", EInputEvent::IE_Pressed, TargetingComp.Get(), &UCTargetingComponent::InputAction_Targeting);
	PlayerInputComponent->BindAction("Menu", EInputEvent::IE_Pressed, GameUIComp.Get(), &UCGameUIComponent::InputAction_ActivateEquipMenu);
	PlayerInputComponent->BindAction("Menu", EInputEvent::IE_Released, GameUIComp.Get(), &UCGameUIComponent::InputAction_DeactivateEquipMenu);
	PlayerInputComponent->BindAction("Action", EInputEvent::IE_Pressed, WeaponComp.Get(), &UCWeaponComponent::InputAction_Act);
	PlayerInputComponent->BindAction("SubAction", EInputEvent::IE_Pressed, WeaponComp.Get(), &UCWeaponComponent::InputAction_SubAct_Pressed);
	PlayerInputComponent->BindAction("SubAction", EInputEvent::IE_Released, WeaponComp.Get(), &UCWeaponComponent::InputAction_SubAct_Released);
	PlayerInputComponent->BindAction("Skill_1", EInputEvent::IE_Pressed, WeaponComp.Get(), &UCWeaponComponent::InputAction_Skill_1_Pressed);
	PlayerInputComponent->BindAction("Skill_1", EInputEvent::IE_Released, WeaponComp.Get(), &UCWeaponComponent::InputAction_Skill_1_Released);
	PlayerInputComponent->BindAction("Skill_2", EInputEvent::IE_Pressed, WeaponComp.Get(), &UCWeaponComponent::InputAction_Skill_2_Pressed);
	PlayerInputComponent->BindAction("Skill_2", EInputEvent::IE_Released, WeaponComp.Get(), &UCWeaponComponent::InputAction_Skill_2_Released);
	PlayerInputComponent->BindAction("Skill_3", EInputEvent::IE_Pressed, WeaponComp.Get(), &UCWeaponComponent::InputAction_Skill_3_Pressed);
	PlayerInputComponent->BindAction("Skill_3", EInputEvent::IE_Released, WeaponComp.Get(), &UCWeaponComponent::InputAction_Skill_3_Released);
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

TObjectPtr<USpringArmComponent> ACPlayableCharacter::GetSpringArm() const
{
	return SpringArm;
}

TObjectPtr<UCTargetingComponent> ACPlayableCharacter::GetTargetingComp() const
{
	return TargetingComp;
}