#include "CPlayableCharacter.h"
#include "Global.h"
#include "Utilities/YJJLocalizedText.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"
#include "Characters/CAnimInstance_Human.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CMontagesComponent.h"
#include "Components/CWeaponComponent.h"
#include "Components/CMagicComponent.h"
#include "Components/CCamComponent.h"
#include "Components/CTargetingComponent.h"
#include "Components/CGameUIComponent.h"
#include "Components/CInventoryComponent.h"
#include "Components/CPlacementComponent.h"
#include "Components/CParkourComponent.h"
#include "Components/CSystemMessageComponent.h"
#include "Commons/CGameMode.h"
#include "Commons/CPlayerController.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/CCharacterStatComponent.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "Widgets/Player/CUserWidget_PlayerBar.h"
#include "Widgets/Player/CUserWidget_PlayerLevel.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Weapons/CUserWidget_EquipMenu.h"
#include "Widgets/Weapons/CUserWidget_MagicMenu.h"
#include "Widgets/Interaction/CUserWidget_Interaction.h"
#include "Widgets/Weapons/CUserWidget_EquipMenuButton.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/ChildActorComponent.h"
#include "NiagaraComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Engine/EngineTypes.h"

namespace
{
	// ActorSequence 컴포넌트는 모듈 의존 없이 SequencePlayer UObject 의 Play 를 반사 호출한다.
	void TryPlayActorSequencePlayer(UActorComponent* SkillSequenceComp)
	{
		if (false == IsValid(SkillSequenceComp))
			return;

		const FProperty* prop = SkillSequenceComp->GetClass()->FindPropertyByName(FName(TEXT("SequencePlayer")));
		const FObjectProperty* objProp = CastField<FObjectProperty>(prop);
		if (objProp == nullptr)
			return;

		UObject* playerObj = objProp->GetObjectPropertyValue_InContainer(SkillSequenceComp);
		if (false == IsValid(playerObj))
			return;

		UFunction* playFn = playerObj->FindFunction(FName(TEXT("Play")));
		if (playFn == nullptr)
			return;

		playerObj->ProcessEvent(playFn, nullptr);
	}
	// WB_Player_* 위젯은 블루프린트 전용 Set*UI 노드라 UFunction 이름만 맞춰 ProcessEvent 로 호출한다.
	void CallWidgetSetHpUi(UUserWidget* Widget, const double InCur, const double InMax)
	{
		if (false == IsValid(Widget))
			return;
		UFunction* fn = Widget->FindFunction(FName(TEXT("SetHPUI")));
		if (fn == nullptr)
			return;
		struct FPay
		{
			double InCur;
			double InMax;
		} p{ InCur, InMax };
		Widget->ProcessEvent(fn, &p);
	}

	void CallWidgetSetStaminaUi(UUserWidget* Widget, const double InCur, const double InMax)
	{
		if (false == IsValid(Widget))
			return;
		UFunction* fn = Widget->FindFunction(FName(TEXT("SetStaminaUI")));
		if (fn == nullptr)
			return;
		struct FPay
		{
			double InCur;
			double InMax;
		} p{ InCur, InMax };
		Widget->ProcessEvent(fn, &p);
	}

	void CallWidgetSetManaUi(UUserWidget* Widget, const double InCur, const double InMax)
	{
		if (false == IsValid(Widget))
			return;
		UFunction* fn = Widget->FindFunction(FName(TEXT("SetManaUI")));
		if (fn == nullptr)
			return;
		struct FPay
		{
			double InCur;
			double InMax;
		} p{ InCur, InMax };
		Widget->ProcessEvent(fn, &p);
	}

	void CallWidgetSetLevelUi(UUserWidget* Widget, const int32 InLevel, const double InCurExp, const double InMaxExp)
	{
		if (false == IsValid(Widget))
			return;
		UFunction* fn = Widget->FindFunction(FName(TEXT("SetLevelUI")));
		if (fn == nullptr)
			return;
		struct FPay
		{
			int32 InLevel;
			double InCurExp;
			double InMaxExp;
		} p{ InLevel, InCurExp, InMaxExp };
		Widget->ProcessEvent(fn, &p);
	}

	void CallWidgetLevelUpAnim(UUserWidget* Widget)
	{
		if (false == IsValid(Widget))
			return;
		UFunction* fn = Widget->FindFunction(FName(TEXT("LevelUp")));
		if (fn == nullptr)
			return;
		Widget->ProcessEvent(fn, nullptr);
	}

	void TryPlaySystemMessage(UCSystemMessageComponent* Comp, const FText& InText, const double InSeconds)
	{
		if (false == IsValid(Comp))
			return;
		Comp->Play(InText, InSeconds);
	}
}

ACPlayableCharacter::ACPlayableCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	{
		static ConstructorHelpers::FClassFinder<UCameraShakeBase> shakeFinder(
			TEXT("/Game/Character/Player/CS_NotEnoughState.CS_NotEnoughState_C"));
		if (shakeFinder.Succeeded())
			NotEnoughStateCameraShakeClass = shakeFinder.Class;
	}

	const TObjectPtr<USkeletalMeshComponent> meshComp = GetMesh();
	const TObjectPtr<UCapsuleComponent> capsuleComp = GetCapsuleComponent();

	// BP_Player: CollisionCylinder — ArrowGroup — (Ceil, Center, Floor, Land_0, Left, Right).
	YJJHelpers::CreateComponent<USceneComponent>(this, &NativeBpArrowGroup, TEXT("YJJ_PlayerArrowGroup"), capsuleComp);
	if (IsValid(NativeBpArrowGroup))
		NativeBpArrowGroup->ComponentTags.Add(FName(TEXT("Arrows")));

	YJJHelpers::CreateComponent<UArrowComponent>(this, &ArrowCeil, TEXT("Ceil"), NativeBpArrowGroup);
	YJJHelpers::CreateComponent<UArrowComponent>(this, &ArrowCenter, TEXT("Center"), NativeBpArrowGroup);
	YJJHelpers::CreateComponent<UArrowComponent>(this, &ArrowFloor, TEXT("Floor"), NativeBpArrowGroup);
	YJJHelpers::CreateComponent<UArrowComponent>(this, &ArrowLand0, TEXT("Land_0"), NativeBpArrowGroup);
	YJJHelpers::CreateComponent<UArrowComponent>(this, &ArrowLeft, TEXT("Left"), NativeBpArrowGroup);
	YJJHelpers::CreateComponent<UArrowComponent>(this, &ArrowRight, TEXT("Right"), NativeBpArrowGroup);

	// BP_Player: CharacterMesh0 — SpringArm — Camera — MainCamChild; SequenceCamera — SequenceCamChild.
	YJJHelpers::CreateComponent<USpringArmComponent>(this, &NativeSpringArm, TEXT("YJJ_PlayerSpringArm"), meshComp);
	YJJHelpers::CreateComponent<UCameraComponent>(this, &NativeCamera, TEXT("YJJ_PlayerCamera"), NativeSpringArm);
	YJJHelpers::CreateComponent<UChildActorComponent>(this, &NativeBpMainCamChild, TEXT("YJJ_PlayerMainCamChild"), NativeCamera);

	YJJHelpers::CreateComponent<USceneComponent>(
		this, &NativeBpSequenceCamAnchor, TEXT("YJJ_PlayerSequenceCamAnchor"), meshComp);
	YJJHelpers::CreateComponent<UChildActorComponent>(
		this, &NativeBpSequenceCamChild, TEXT("YJJ_PlayerSequenceCamChild"), NativeBpSequenceCamAnchor);

	// 이름이 너무 짧으면 레거시 BP_Player SCS 의 Scene / PointLight 슬롯과 충돌해 인터페이스 제거 등으로 재구성 트리거 시 Fatal 할 수 있다.
	YJJHelpers::CreateComponent<USceneComponent>(this, &CinematicLightScene, TEXT("YJJ_CinematicLightRoot"), meshComp);
	YJJHelpers::CreateComponent<UPointLightComponent>(
		this,
		&NativeBpCinematicPointLightA,
		TEXT("YJJ_CinematicPointLightA"),
		CinematicLightScene);
	YJJHelpers::CreateComponent<UPointLightComponent>(
		this,
		&NativeBpCinematicPointLightB,
		TEXT("YJJ_CinematicPointLightB"),
		CinematicLightScene);

	YJJHelpers::CreateActorComponent<UCWeaponComponent>(this, &WeaponComp, TEXT("YJJ_PlayerWeaponComp"));
	NativeBpWeaponAlias = WeaponComp;
	YJJHelpers::CreateActorComponent<UCMagicComponent>(this, &MagicComp, TEXT("YJJ_PlayerMagicComp"));
	NativeBpMagicAlias = MagicComp;
	// 레거시 BP_Player SCS 가 CamComponent/ParkourComponent 등 과 동일 이름의 BPGC 슬롯을 남기면, 네이티브 서브오브젝트와 이름·클래스가 겹쳐 RemoveInterface 후 재구성 시 Fatal 될 수 있어 접두를 둔다.
	YJJHelpers::CreateActorComponent<UCCamComponent>(this, &CamComp, TEXT("YJJ_PlayerCamComp"));
	YJJHelpers::CreateActorComponent<UCTargetingComponent>(this, &TargetingComp, TEXT("YJJ_PlayerTargetingComp"));
	NativeBpTargetingAlias = TargetingComp;
	YJJHelpers::CreateActorComponent<UCGameUIComponent>(this, &GameUIComp, TEXT("YJJ_PlayerGameUIComp"));
	YJJHelpers::CreateActorComponent<UCInventoryComponent>(this, &InventoryComp, TEXT("YJJ_PlayerInventoryComp"));
	YJJHelpers::CreateActorComponent<UCPlacementComponent>(this, &PlacementComp, TEXT("YJJ_PlayerPlacementComp"));
	YJJHelpers::CreateActorComponent<UCParkourComponent>(this, &ParkourComp, TEXT("YJJ_PlayerParkourComp"));
	YJJHelpers::CreateActorComponent<UCSystemMessageComponent>(
		this, &NativeBpSystemMessageComp, TEXT("YJJ_PlayerSystemMessage"));

	TObjectPtr<USkeletalMesh> mesh = nullptr;
	YJJHelpers::GetAsset<USkeletalMesh>(&mesh, "SkeletalMesh'/Game/Assets/Character/MercenaryWarrior/Meshes/SK_MercenaryWarrior_WithoutHelmet.SK_MercenaryWarrior_WithoutHelmet'");

	GetMesh()->SetSkeletalMeshAsset(mesh.Get());
	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	TSubclassOf<UCAnimInstance_Human> animInstance;
	YJJHelpers::GetClass<UCAnimInstance_Human>(&animInstance, "AnimBlueprint'/Game/Character/CABP_Human.CABP_Human_C'");
	GetMesh()->SetAnimInstanceClass(animInstance);

	if (IsValid(NativeSpringArm))
	{
		NativeSpringArm->SetRelativeLocation(FVector(0, 0, 60));
		NativeSpringArm->TargetArmLength = 280;
		NativeSpringArm->bUsePawnControlRotation = true;
		NativeSpringArm->bEnableCameraLag = true;
		NativeSpringArm->bDoCollisionTest = false;
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

void ACPlayableCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// CBP_PlayableCharacter 의 CharacterMesh0 가 abstract UCAnimInstance_Character 로 저장된 경우
	// OnRegister(AnimInstance 생성) 전에 구체 AnimBP 로 교정한다. ctor 의 SetAnimInstanceClass 는 BP 기본값에 덮인다.
	USkeletalMeshComponent* const mesh = GetMesh();
	if (false == IsValid(mesh))
		return;

	UClass* const currentAnimClass = mesh->GetAnimClass();
	if (IsValid(currentAnimClass) && false == currentAnimClass->HasAnyClassFlags(CLASS_Abstract))
		return;

	TSubclassOf<UCAnimInstance_Human> animInstanceClass;
	YJJHelpers::GetClassDynamic<UCAnimInstance_Human>(
		&animInstanceClass,
		TEXT("/Game/Character/CABP_Human.CABP_Human_C"));

	if (IsValid(animInstanceClass))
	{
		mesh->SetAnimInstanceClass(animInstanceClass);
		return;
	}

	CLog::Log(FString::Printf(
		TEXT("[Anim] PostInitializeComponents: CABP_Human 로드 실패 — CharacterMesh AnimClass=%s Actor=%s"),
		IsValid(currentAnimClass) ? *currentAnimClass->GetName() : TEXT("(null)"),
		*GetName()));
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
	if (false == IsValid(yjjPlayerController))
		yjjPlayerController = Cast<ACPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

	if (IsLocallyControlled())
	{
		if (IsValid(yjjPlayerController))
			yjjPlayerController->InitializeHUDForPawn(this);
		else
			CLog::Log(FString::Printf(
				TEXT("[UI] CPlayableCharacter::BeginPlay: 로컬 폰 HUD 초기화 실패 — ACPlayerController 없음(Project Settings → Game Mode 의 Player Controller Class 확인) (%s)"),
				*GetName()));
	}

	if (IsValid(CharacterInfoComp))
		CharacterInfoComp->SetCharacterType(CECharacterType::Player);

	// 블루프린트 줌 변수(ZoomData/Zooming)를 CamComponent 초깃값과 맞춘다.
	if (IsValid(CamComp))
	{
		ZoomData = CamComp->ZoomData;
		Zooming = static_cast<double>(CamComp->Zooming);
	}

	// 레거시 BP Reward/Status 변수와 CharacterStatComp 초기값을 맞춘다.
	if (IsValid(CharacterStatComp))
	{
		Level = CharacterStatComp->CurLevel;
		Exp = static_cast<double>(CharacterStatComp->GetCurExp());
		MaxExp = static_cast<double>(CharacterStatComp->GetMaxExp());
		Hp = static_cast<double>(CharacterStatComp->GetCurHp());
		MaxHp = static_cast<double>(CharacterStatComp->GetMaxHp());
		Stamina = static_cast<double>(CharacterStatComp->GetCurStamina());
		MaxStamina = static_cast<double>(CharacterStatComp->GetMaxStamina());
		Mana = static_cast<double>(CharacterStatComp->GetCurMana());
		MaxMana = static_cast<double>(CharacterStatComp->GetMaxMana());
	}

	OriginZooming = Zooming;

	UCWeaponComponent* const resolvedWeapon = EnsureWeaponComp();
	if (IsValid(resolvedWeapon))
	{
		resolvedWeapon->EnsureWeaponPipelineReady();
		resolvedWeapon->SyncBpWeaponLanes();
		resolvedWeapon->EnsureCombatWeaponEquipped();
		resolvedWeapon->LogWeaponPipelineStatus(TEXT("PlayableBeginPlay"));
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon] PlayableCharacter::BeginPlay — UCWeaponComponent 없음 Actor=%s"),
			*GetName()));
	}
}

UCWeaponComponent* ACPlayableCharacter::EnsureWeaponComp()
{
	if (IsValid(WeaponComp) && (WeaponComp->GetOwner() == this))
		return WeaponComp.Get();

	TArray<UCWeaponComponent*> weaponComponents;
	GetComponents<UCWeaponComponent>(weaponComponents);

	UCWeaponComponent* resolved = nullptr;
	const int32 componentCount = weaponComponents.Num();
	for (int32 componentIndex = 0; componentIndex < componentCount; ++componentIndex)
	{
		UCWeaponComponent* const candidate = weaponComponents[componentIndex];
		if (false == IsValid(candidate))
			continue;
		if (candidate->GetFName() == FName(TEXT("YJJ_PlayerWeaponComp")))
		{
			resolved = candidate;
			break;
		}
	}

	if (false == IsValid(resolved) && componentCount > 0)
		resolved = weaponComponents[0];

	if (IsValid(resolved))
	{
		WeaponComp = resolved;
		NativeBpWeaponAlias = resolved;

		if (componentCount > 1)
		{
			CLog::Log(FString::Printf(
				TEXT("[Weapon] WeaponComp %d개 — YJJ_PlayerWeaponComp=%s 사용 Actor=%s"),
				componentCount,
				*resolved->GetName(),
				*GetName()));
		}
	}

	return resolved;
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

	HpBar = Cast<UUserWidget>(playerInfo->BoundHpBar.Get());
	StaminaBar = Cast<UUserWidget>(playerInfo->BoundStaminaBar.Get());
	ManaBar = Cast<UUserWidget>(playerInfo->BoundManaBar.Get());
	LevelBar = Cast<UUserWidget>(playerInfo->BoundLevelBar.Get());

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
		EquipMenu = equipMenu;
		equipMenu->SetVisibility(ESlateVisibility::Hidden);
		// SetMenuUI 가 여러 번 호출돼도 동일 핸들러가 중복되지 않게 먼저 제거한다.
		equipMenu->OnWeaponEquipped.RemoveDynamic(this, &ACPlayableCharacter::EquipWeaponFromUI);
		equipMenu->OnWeaponEquipped.AddDynamic(this, &ACPlayableCharacter::EquipWeaponFromUI);

		for (int32 buttonIndex = 0; buttonIndex < equipMenu->EquipMenuButtons.Num(); buttonIndex++)
		{
			UCUserWidget_EquipMenuButton* equipButton = equipMenu->EquipMenuButtons[buttonIndex].Get();
			if (IsValid(equipButton))
			{
				equipButton->OnWeaponTypeHovered.RemoveDynamic(this, &ACPlayableCharacter::OnEquipMenuWeaponHoveredBridge);
				equipButton->OnWeaponTypeHovered.AddUniqueDynamic(this, &ACPlayableCharacter::OnEquipMenuWeaponHoveredBridge);
				equipButton->OnWeaponTypeUnhovered.RemoveDynamic(this, &ACPlayableCharacter::OnEquipMenuWeaponUnhoveredBridge);
				equipButton->OnWeaponTypeUnhovered.AddUniqueDynamic(this, &ACPlayableCharacter::OnEquipMenuWeaponUnhoveredBridge);
			}
		}
	}
	else
		CLog::Log(FString::Printf(TEXT("[UI] SetMenuUI: EquipMenu(CEquipMenu) 없음 — %s"), *GetName()));

	UCUserWidget_MagicMenu* magicMenu = hud->GetMagicMenuWidget();
	if (IsValid(magicMenu))
	{
		MenuMagicWidget = magicMenu;
		MagicMenu = magicMenu;
		magicMenu->SetVisibility(ESlateVisibility::Hidden);
		magicMenu->OnEquipMagic.RemoveDynamic(this, &ACPlayableCharacter::EquipMagicFromUI);
		magicMenu->OnEquipMagic.AddDynamic(this, &ACPlayableCharacter::EquipMagicFromUI);
	}
	else
		CLog::Log(FString::Printf(TEXT("[UI] SetMenuUI: MagicMenu(CMagicMenu) 없음 — HUD에 WB_MagicMenu 를 넣고 Parent=UCUserWidget_MagicMenu, 이름 CMagicMenu — %s"), *GetName()));

	UCUserWidget_Interaction* interactionWidget = hud->GetInteractionWidget();
	if (IsValid(interactionWidget))
	{
		MenuInteractionWidget = interactionWidget;
		Interaction = interactionWidget;
	}
	else
		CLog::Log(FString::Printf(TEXT("[UI] SetMenuUI: Interaction(CInteraction) 없음 — HUD=%s Actor=%s"),
			IsValid(hud) ? *hud->GetClass()->GetName() : TEXT("(null)"),
			*GetName()));
}

void ACPlayableCharacter::SetupCinematic_Implementation(bool OnOff)
{
	// I_Cinematic 포팅 기본 구현: BP_Player 가 오버라이드해 실제 연출을 채운다.
}

void ACPlayableCharacter::EquipWeaponFromUI(const CEWeaponType InNewType)
{
	UCWeaponComponent* const weapon = EnsureWeaponComp();
	if (false == IsValid(weapon))
	{
		CLog::Log(TEXT("[UI] EquipWeaponFromUI: WeaponComp 없음"));
		return;
	}

	weapon->EnsureWeaponPipelineReady();
	weapon->SetMode(InNewType);
}

void ACPlayableCharacter::EquipMagicFromUI(const CEMagicType InNewType)
{
	UCWeaponComponent* const weapon = EnsureWeaponComp();
	if (false == IsValid(weapon))
	{
		CLog::Log(TEXT("[UI] EquipMagicFromUI: WeaponComp 없음"));
		return;
	}

	weapon->EnsureWeaponPipelineReady();
	weapon->SetMagicMode(InNewType);
}

void ACPlayableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Tick_AirBone();
	Tick_AccelGravity();
	Tick_LerpMove(DeltaTime);
	Tick_Fluid();
}

void ACPlayableCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UCWeaponComponent* const resolvedWeapon = EnsureWeaponComp();

	UCMovementComponent* const resolvedMovement = EnsureMovementComp();
	if (IsValid(resolvedMovement))
	{
		PlayerInputComponent->BindAxis("MoveForward", resolvedMovement, &UCMovementComponent::InputAxis_MoveForward);
		PlayerInputComponent->BindAxis("MoveRight", resolvedMovement, &UCMovementComponent::InputAxis_MoveRight);

		PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Pressed, resolvedMovement, &UCMovementComponent::InputAction_Walk);
		PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Released, resolvedMovement, &UCMovementComponent::InputAction_Run);
		PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, resolvedMovement, &UCMovementComponent::InputAction_Jump);
	}
	else if (IsLocallyControlled())
	{
		CLog::Log(FString::Printf(
			TEXT("[입력 바인딩] MovementComp 없음 — MoveForward/Walk/Jump 미바인딩. BP 레거시 컴포넌트 중복 여부 확인. Actor=%s"),
			*GetNameSafe(this)));
	}

	PlayerInputComponent->BindAxis("HorizontalLook", CamComp.Get(), &UCCamComponent::InputAxis_HorizontalLook);
	PlayerInputComponent->BindAxis("VerticalLook", CamComp.Get(), &UCCamComponent::InputAxis_VerticalLook);
	PlayerInputComponent->BindAxis("Zoom", CamComp.Get(), &UCCamComponent::InputAxis_Zoom);
	PlayerInputComponent->BindAction("Avoid", EInputEvent::IE_Pressed, this, &ACPlayableCharacter::InputAction_Avoid);
	PlayerInputComponent->BindAction("Targeting", EInputEvent::IE_Pressed, TargetingComp.Get(), &UCTargetingComponent::InputAction_Targeting);
	PlayerInputComponent->BindAction("Menu", EInputEvent::IE_Pressed, GameUIComp.Get(), &UCGameUIComponent::InputAction_ActivateEquipMenu);
	PlayerInputComponent->BindAction("Menu", EInputEvent::IE_Released, GameUIComp.Get(), &UCGameUIComponent::InputAction_DeactivateEquipMenu);
	if (IsValid(resolvedWeapon))
	{
		PlayerInputComponent->BindAction("Action", EInputEvent::IE_Pressed, resolvedWeapon, &UCWeaponComponent::InputAction_Act);
	}
	else if (IsLocallyControlled())
	{
		CLog::Log(FString::Printf(
			TEXT("[입력 바인딩] WeaponComp 없음 — Action(좌클릭) 미바인딩. BP_Player 가 YJJ_PlayerWeaponComp 를 덮었는지 확인. Actor=%s"),
			*GetNameSafe(this)));
	}

	if (IsValid(resolvedWeapon))
	{
		PlayerInputComponent->BindAction("SubAction", EInputEvent::IE_Pressed, resolvedWeapon, &UCWeaponComponent::InputAction_SubAct_Pressed);
		PlayerInputComponent->BindAction("SubAction", EInputEvent::IE_Released, resolvedWeapon, &UCWeaponComponent::InputAction_SubAct_Released);
		PlayerInputComponent->BindAction("Skill_1", EInputEvent::IE_Pressed, resolvedWeapon, &UCWeaponComponent::InputAction_Skill_1_Pressed);
		PlayerInputComponent->BindAction("Skill_1", EInputEvent::IE_Released, resolvedWeapon, &UCWeaponComponent::InputAction_Skill_1_Released);
		PlayerInputComponent->BindAction("Skill_2", EInputEvent::IE_Pressed, resolvedWeapon, &UCWeaponComponent::InputAction_Skill_2_Pressed);
		PlayerInputComponent->BindAction("Skill_2", EInputEvent::IE_Released, resolvedWeapon, &UCWeaponComponent::InputAction_Skill_2_Released);
		PlayerInputComponent->BindAction("Skill_3", EInputEvent::IE_Pressed, resolvedWeapon, &UCWeaponComponent::InputAction_Skill_3_Pressed);
		PlayerInputComponent->BindAction("Skill_3", EInputEvent::IE_Released, resolvedWeapon, &UCWeaponComponent::InputAction_Skill_3_Released);
	}
}

void ACPlayableCharacter::InputAction_Avoid()
{
	CheckNull(StateComp);

	// IsIdleOnly(IsIdleMode) 이면 Equip/Fall 등에서 Shift 회피가 전부 차단된다. 레거시 IsMoveable + 공중은 허용.
	// bCanMove(CheckFalse MovementComp::CanMove) 는 Stop()·일시 봉인과 무관하게 회피까지 막으므로 쓰지 않는다.
	if (false == StateComp->IsMoveable() && false == StateComp->IsFallMode())
		return;

	StateComp->SetAvoidMode();
}

void ACPlayableCharacter::Avoid()
{
	CheckNull(MontagesComp);
	MontagesComp->PlayAvoidAnim();
}

void ACPlayableCharacter::Hit()
{
	CurHitType = CEHitReactionFromAttackType(Damage.Event.HitData.AttackType);

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
	return NativeSpringArm;
}

TObjectPtr<UCTargetingComponent> ACPlayableCharacter::GetTargetingComp() const
{
	return TargetingComp;
}

bool ACPlayableCharacter::IsBowMode() const
{
	return IsValid(WeaponComp) && WeaponComp->IsBowMode();
}

void ACPlayableCharacter::SetDamage(const float InDamage, bool& OutHittedOrDead)
{
	OutHittedOrDead = false;

	float dmg = InDamage;
	if (dmg <= KINDA_SMALL_NUMBER)
		dmg = HitData.Damage;

	if (dmg <= KINDA_SMALL_NUMBER)
		return;

	Hp = FMath::Clamp(static_cast<double>(Hp) - static_cast<double>(dmg), 0.0, MaxHp);

	if (IsValid(CharacterStatComp))
		CharacterStatComp->SetHp(static_cast<float>(Hp));

	UpdateHp();
	OutHittedOrDead = Hp > 0.0;
}

void ACPlayableCharacter::CanHitAnim(bool& OutCanHitAnim)
{
	OutCanHitAnim = false;
	if (false == IsValid(StateComp))
		return;

	if (StateComp->IsDeadMode())
		return;

	// BP: Hitted/HitAir/DownFlying/DownLand 분기에서 End_Hitted 후 true — C++ 는 피격 타입 활성으로 근사.
	if (StateComp->IsHitted())
	{
		End_Hitted();
		OutCanHitAnim = true;
		return;
	}

	const CEStateType prev = StateComp->GetPrevMode();
	switch (prev)
	{
	case CEStateType::Idle:
	case CEStateType::Rise:
	case CEStateType::Riding:
		OutCanHitAnim = true;
		return;
	case CEStateType::Equip:
	case CEStateType::Act:
	case CEStateType::Fall:
	case CEStateType::Avoid:
	case CEStateType::Land:
	case CEStateType::Dead:
	default:
		OutCanHitAnim = false;
		return;
	}
}

void ACPlayableCharacter::CheckGruadOrParrying(bool& OutResult)
{
	OutResult = false;
	if (false == IsValid(WeaponComp))
		return;

	FObjectProperty* subProp = CastField<FObjectProperty>(
		WeaponComp->GetClass()->FindPropertyByName(FName(TEXT("SubWeapon"))));
	if (subProp == nullptr)
		return;

	void* subSlot = subProp->ContainerPtrToValuePtr<void>(WeaponComp);
	UObject* subObj = subProp->GetObjectPropertyValue(subSlot);
	AActor* subActor = Cast<AActor>(subObj);
	if (false == IsValid(subActor))
		return;

	bool bGuarding = false;
	bool bParrying = false;

	FBoolProperty* guardProp = CastField<FBoolProperty>(
		subActor->GetClass()->FindPropertyByName(FName(TEXT("Guarding"))));
	if (guardProp != nullptr)
	{
		bGuarding = guardProp->GetPropertyValue(guardProp->ContainerPtrToValuePtr<void>(subActor));
	}

	FBoolProperty* parryProp = CastField<FBoolProperty>(
		subActor->GetClass()->FindPropertyByName(FName(TEXT("Parrying"))));
	if (parryProp != nullptr)
	{
		bParrying = parryProp->GetPropertyValue(parryProp->ContainerPtrToValuePtr<void>(subActor));
	}

	OutResult = bGuarding || bParrying;
}

void ACPlayableCharacter::PlayHitAnim()
{
	if (HitData.Montage == nullptr)
		return;

	PlayAnimMontage(HitData.Montage, HitData.PlayRate);
}

void ACPlayableCharacter::CancelHitAnim()
{
	if (false == IsValid(StateComp))
		return;
	if (false == StateComp->IsHitted())
		return;

	End_Hitted();

	// BP 기본 무명 핀: HitReaction_Stop_Montage
	UAnimMontage* stopMontage = LoadObject<UAnimMontage>(
		nullptr,
		TEXT("/Game/Character/Player/Montages/Common/HitReaction_Stop_Montage.HitReaction_Stop_Montage"));
	StopAnimMontage(stopMontage);
}

void ACPlayableCharacter::SpawnMessage()
{
	TryPlaySystemMessage(
		NativeBpSystemMessageComp.Get(),
		YJJLocalization::LocalizedText_PlayerSpawnedNotice(),
		5.0);
}

void ACPlayableCharacter::SetMinimap()
{
	if (nullptr == Minimap.Get())
		return;

	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return;

	// UI/표시용 액터는 데디 전용에서 생성하지 않는다.
	if (world->GetNetMode() == NM_DedicatedServer)
		return;

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;
	world->SpawnActor<AActor>(Minimap, GetActorTransform(), params);
}

UCUserWidget_HUD* ACPlayableCharacter::GetHUD()
{
	return EnsureHUDWidget();
}

void ACPlayableCharacter::SetColor()
{
	const FLinearColor flashColor(1.0f, 0.0f, 0.0f, 1.0f);
	const int32 count = Materials.Num();
	for (int32 index = 0; index < count; index++)
	{
		UMaterialInstanceDynamic* mid = Materials[index].Get();
		if (IsValid(mid))
			mid->SetVectorParameterValue(FName(TEXT("BodyColor")), flashColor);
	}

	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return;

	world->GetTimerManager().ClearTimer(BodyColorRestoreTimerHandle);
	world->GetTimerManager().SetTimer(
		BodyColorRestoreTimerHandle,
		FTimerDelegate::CreateUObject(this, &ACPlayableCharacter::SetOriginColor),
		0.15f,
		false);
}

void ACPlayableCharacter::SetOriginColor()
{
	const int32 count = Materials.Num();
	for (int32 index = 0; index < count; index++)
	{
		UMaterialInstanceDynamic* mid = Materials[index].Get();
		if (IsValid(mid))
			mid->SetVectorParameterValue(FName(TEXT("BodyColor")), CharacterInfo.BodyColor);
	}
}

void ACPlayableCharacter::LoadPrevState()
{
	if (IsValid(StateComp) && StateComp->IsDead())
		return;

	ApplyRestoreStateFromPrevMode();
}

void ACPlayableCharacter::UpdateLevel()
{
	CallWidgetLevelUpAnim(Cast<UUserWidget>(LevelBar.Get()));
}

void ACPlayableCharacter::UpdateHp()
{
	CallWidgetSetHpUi(Cast<UUserWidget>(HpBar.Get()), Hp, MaxHp);
}

void ACPlayableCharacter::UpdateStamina()
{
	CallWidgetSetStaminaUi(Cast<UUserWidget>(StaminaBar.Get()), Stamina, MaxStamina);
}

void ACPlayableCharacter::UpdateMana()
{
	CallWidgetSetManaUi(Cast<UUserWidget>(ManaBar.Get()), Mana, MaxMana);
}

void ACPlayableCharacter::LevelUp()
{
	Level = Level + 1;
	Exp = FMath::Max(0.0, Exp - MaxExp);
}

void ACPlayableCharacter::UpdateExp()
{
	CallWidgetSetLevelUi(Cast<UUserWidget>(LevelBar.Get()), Level, Exp, MaxExp);
}

void ACPlayableCharacter::SetMaterial()
{
	Materials.Empty();
	USkeletalMeshComponent* mesh = GetMesh();
	if (false == IsValid(mesh))
		return;

	const int32 slotCount = mesh->GetNumMaterials();
	for (int32 slotIndex = 0; slotIndex < slotCount; slotIndex++)
	{
		UMaterialInterface* parent = mesh->GetMaterial(slotIndex);
		if (false == IsValid(parent))
			continue;

		UMaterialInstanceDynamic* mid = UMaterialInstanceDynamic::Create(parent, this);
		if (false == IsValid(mid))
			continue;

		mid->SetVectorParameterValue(FName(TEXT("BodyColor")), CharacterInfo.BodyColor);
		mesh->SetMaterial(slotIndex, mid);
		Materials.Add(mid);
	}
}

void ACPlayableCharacter::SetStatus()
{
	Hp = MaxHp;
	Stamina = MaxStamina;
	Mana = MaxMana;

	if (IsValid(CharacterStatComp))
	{
		CharacterStatComp->SetHp(static_cast<float>(Hp));
		CharacterStatComp->SetStamina(static_cast<float>(Stamina));
		CharacterStatComp->SetMana(static_cast<float>(Mana));
	}
}

void ACPlayableCharacter::SetViewPitch()
{
	const TWeakObjectPtr<APlayerController> pc = Cast<APlayerController>(GetController());
	if (false == pc.IsValid())
		return;
	if (false == IsValid(pc->PlayerCameraManager))
		return;

	pc->PlayerCameraManager->ViewPitchMin = PitchRange.X;
	pc->PlayerCameraManager->ViewPitchMax = PitchRange.Y;
}

void ACPlayableCharacter::RestoreStamina()
{
	if (false == IsValid(StateComp))
		return;

	const bool bAllowRestore = StateComp->IsIdle() || StateComp->IsRidingRecoverContext();
	if (false == bAllowRestore)
	{
		AccelStaminaRestore = 1.0;
		return;
	}

	if (false == (Stamina < MaxStamina))
		return;

	const double delta = DefaultStaminaRestore * AccelStaminaRestore;
	Stamina = FMath::Clamp(Stamina + delta, 0.0, MaxStamina);
	AccelStaminaRestore *= 1.1;

	if (IsValid(CharacterStatComp))
		CharacterStatComp->SetStamina(static_cast<float>(Stamina));

	UpdateStamina();

	if (FMath::IsNearlyEqual(Stamina, MaxStamina, 0.01))
		UKismetSystemLibrary::K2_PauseTimer(this, FString(TEXT("RestoreStamina")));
}

void ACPlayableCharacter::RestoreMana()
{
	if (false == IsValid(StateComp))
		return;

	const bool bAllowRestore = StateComp->IsIdle() || StateComp->IsRidingRecoverContext();
	if (false == bAllowRestore)
	{
		AccelManaRestore = 1.0;
		return;
	}

	if (false == (Mana < MaxMana))
		return;

	const double delta = DefaultManaRestore * AccelManaRestore;
	Mana = FMath::Clamp(Mana + delta, 0.0, MaxMana);
	AccelManaRestore *= 1.05;

	if (IsValid(CharacterStatComp))
		CharacterStatComp->SetMana(static_cast<float>(Mana));

	UpdateMana();

	if (FMath::IsNearlyEqual(Mana, MaxMana, 0.01))
		UKismetSystemLibrary::K2_PauseTimer(this, FString(TEXT("RestoreMana")));
}

bool ACPlayableCharacter::IsEnoughStamina(const double InConsume)
{
	const bool bEnough = Stamina >= InConsume;
	EnoughStamina = bEnough;
	return bEnough;
}

bool ACPlayableCharacter::ConsumeStamina(const double InConsume)
{
	if (InConsume <= 0.0)
	{
		EnoughStamina = true;
		return true;
	}

	const bool bEnough = Stamina >= InConsume;
	EnoughStamina = bEnough;
	if (false == bEnough)
	{
		NotEnoughStamina();
		return false;
	}

	Stamina = FMath::Max(0.0, Stamina - InConsume);

	if (IsValid(CharacterStatComp))
		CharacterStatComp->SetStamina(static_cast<float>(Stamina));

	UpdateStamina();

	return true;
}

bool ACPlayableCharacter::ConsumeMana(double InConsume)
{
	if (InConsume <= 0.0)
	{
		EnoughMana = true;
		return true;
	}

	const bool bEnough = IsEnoughMana(InConsume);
	if (false == bEnough)
	{
		NotEnoughMana();
		return false;
	}

	Mana = FMath::Clamp(Mana - InConsume, 0.0, MaxMana);

	if (IsValid(CharacterStatComp))
		CharacterStatComp->SetMana(static_cast<float>(Mana));

	UpdateMana();

	return true;
}

bool ACPlayableCharacter::IsEnoughMana(const double InConsume)
{
	const bool bEnough = Mana >= InConsume;
	EnoughMana = bEnough;
	return bEnough;
}

void ACPlayableCharacter::ShakeCam_Implementation()
{
	// BP_Player::ShakeCam — 로컬 PlayerCameraManager 에 CS_NotEnoughState (기본 클래스 프로퍼티).
	if (false == IsLocallyControlled())
		return;

	APlayerController* playerController = Cast<APlayerController>(GetController());
	if (false == IsValid(playerController) || false == IsValid(playerController->PlayerCameraManager))
		return;

	if (NotEnoughStateCameraShakeClass == nullptr)
		return;

	playerController->PlayerCameraManager->StartCameraShake(
		NotEnoughStateCameraShakeClass,
		1.0f,
		ECameraShakePlaySpace::CameraLocal,
		FRotator::ZeroRotator);
}

void ACPlayableCharacter::NotEnoughStamina()
{
	ShakeCam();
	TryPlaySystemMessage(NativeBpSystemMessageComp.Get(), YJJLocalization::LocalizedText_NotEnough_Stamina(), 3.0);
	SetIdle();
}

void ACPlayableCharacter::NotEnoughMana()
{
	ShakeCam();
	TryPlaySystemMessage(NativeBpSystemMessageComp.Get(), YJJLocalization::LocalizedText_NotEnough_Mana(), 3.0);
	SetIdle();
}

void ACPlayableCharacter::SetDefaultController()
{
	CurController = GetController();
}

void ACPlayableCharacter::SetZooming(double InZooming)
{
	Zooming = InZooming;
	if (IsValid(CamComp))
	{
		CamComp->ZoomData = ZoomData;
		CamComp->SetZooming(static_cast<float>(InZooming));
	}
}

void ACPlayableCharacter::SaveZooming()
{
	OriginZooming = Zooming;
}

void ACPlayableCharacter::SetSkillZooming()
{
	Zooming = SkillZooming;
}

void ACPlayableCharacter::ApplyZoom(double InZoom)
{
	// BP: TargetArmLength 가 InZoom 과 거의 같지 않을 때만 FInterpTo 로 갱신.
	if (false == IsValid(NativeSpringArm))
		return;

	const float current = NativeSpringArm->TargetArmLength;
	if (UKismetMathLibrary::NearlyEqual_FloatFloat(static_cast<double>(current), InZoom, 0.1))
		return;

	const float deltaSeconds = UGameplayStatics::GetWorldDeltaSeconds(this);
	const float nextLength = UKismetMathLibrary::FInterpTo(
		current,
		static_cast<float>(InZoom),
		deltaSeconds,
		ZoomData.InterpSpeed);

	NativeSpringArm->TargetArmLength = nextLength;
}

void ACPlayableCharacter::StartFall(double InGravity)
{
	if (IsValid(MovementComp))
		MovementComp->SetGravity(static_cast<float>(InGravity));

	UCharacterMovementComponent* characterMovement = GetCharacterMovement();
	if (IsValid(characterMovement))
		characterMovement->SetMovementMode(MOVE_Falling);

	if (IsValid(StateComp))
		StateComp->SetFalling();

	FlyToFall = true;
}

bool ACPlayableCharacter::IsChangedLandCoord() const
{
	const FVector location = GetActorLocation();
	constexpr double tolerance = 50.0;
	const bool nearX = UKismetMathLibrary::NearlyEqual_FloatFloat(location.X, CoordBeforeAir.X, tolerance);
	const bool nearY = UKismetMathLibrary::NearlyEqual_FloatFloat(location.Y, CoordBeforeAir.Y, tolerance);
	const bool nearZ = UKismetMathLibrary::NearlyEqual_FloatFloat(location.Z, CoordBeforeAir.Z, tolerance);
	return nearX || nearY || nearZ;
}

void ACPlayableCharacter::Tick_AirBone()
{
	UCharacterMovementComponent* characterMovement = GetCharacterMovement();
	if (false == IsValid(characterMovement))
		return;

	// BP Tick_AirBone: IsFlying 은 CharacterMovement 기준(블루프린트는 StateComponent 에 묶여 있었으나 컴파일 실패).
	if (characterMovement->IsFlying())
		return;

	if (false == characterMovement->IsFalling())
		return;

	// FlyToFall 이면 높이 검사 분기로 가지 않음(연출 1회).
	if (FlyToFall)
		return;

	const FVector location = GetActorLocation();
	const double targetZ = CoordBeforeAir.Z + AirDistance;
	if (UKismetMathLibrary::NearlyEqual_FloatFloat(location.Z, targetZ, 50.0))
		StartFall(2.0);
}

void ACPlayableCharacter::SetCoordBeforeAir()
{
	CoordBeforeAir = GetActorLocation();
}

void ACPlayableCharacter::OnEquipMenuWeaponHoveredBridge(const CEWeaponType InType)
{
	OnEquipMenuWeaponHovered(InType);
}

void ACPlayableCharacter::OnEquipMenuWeaponUnhoveredBridge(const CEWeaponType InType)
{
	OnEquipMenuWeaponUnhovered(InType);
}

void ACPlayableCharacter::OnEquipMenuWeaponHovered_Implementation(const CEWeaponType InType)
{
	// BP I_Character 의 HoveredEquipMenu 를 BP_Player 에서 오버라이드해 채운다.
	(void)InType;
}

void ACPlayableCharacter::OnEquipMenuWeaponUnhovered_Implementation(const CEWeaponType InType)
{
	(void)InType;
}

void ACPlayableCharacter::Tick_LerpMove(float DeltaTime)
{
	// 위치 보간은 캐릭터 권위와 동일하게만 적용한다.
	if (false == HasAuthority())
		return;

	if (false == IsValid(MovementComp) || false == MovementComp->IsLerpMove())
		return;

	const FVector current = GetActorLocation();
	const FVector dest = MovementComp->Dest;
	const FVector newLoc = FMath::VInterpTo(current, dest, DeltaTime, MovementComp->InterpSpeed);

	SetActorLocation(newLoc, false, nullptr, ETeleportType::None);

	const float arrivalToleranceResolved = MovementComp->LerpArrivalXYTolerance;
	const bool bNearX = FMath::IsNearlyEqual(newLoc.X, dest.X, arrivalToleranceResolved);
	const bool bNearY = FMath::IsNearlyEqual(newLoc.Y, dest.Y, arrivalToleranceResolved);
	if (bNearX && bNearY)
		MovementComp->SetLerpMove(false);
}

bool ACPlayableCharacter::Tick_CheckGround() const
{
	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return false;

	const FVector start = GetActorLocation();
	FVector end = start;
	end.Z -= 300.f;

	FHitResult hit;
	const bool bHit = UKismetSystemLibrary::LineTraceSingle(
		const_cast<ACPlayableCharacter*>(this),
		start,
		end,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		hit,
		true);

	return bHit;
}

void ACPlayableCharacter::Tick_AccelGravity()
{
	if (false == HasAuthority())
		return;

	// BP: 지면 트레이스에 맞으면 분기 없음, 아니면 중력 3.
	if (Tick_CheckGround())
		return;

	if (IsValid(MovementComp))
		MovementComp->SetGravity(3.f);
}

void ACPlayableCharacter::SetInvisibleMotionTrail()
{
	if (false == IsValid(NativeBpMotionTrailNiagara))
		return;

	if (NativeBpMotionTrailNiagara->IsVisible())
		NativeBpMotionTrailNiagara->SetVisibility(false, false);
}

void ACPlayableCharacter::SetVisibleMotionTrail()
{
	if (false == IsValid(NativeBpMotionTrailNiagara))
		return;

	if (false == NativeBpMotionTrailNiagara->IsVisible())
		NativeBpMotionTrailNiagara->SetVisibility(true, false);
}

void ACPlayableCharacter::Begin_SkillCam()
{
	if (false == IsLocallyControlled())
		return;

	OriginZooming = Zooming;
	SetZooming(200.0);

	APlayerController* pc = Cast<APlayerController>(GetController());
	if (false == IsValid(pc))
		pc = UGameplayStatics::GetPlayerController(this, 0);

	if (IsValid(NativeBpSequenceCamChild))
	{
		AActor* camActor = NativeBpSequenceCamChild->GetChildActor();
		if (IsValid(camActor) && IsValid(pc))
		{
			pc->SetViewTargetWithBlend(
				camActor,
				0.2f,
				EViewTargetBlendFunction::VTBlend_EaseInOut,
				1.f,
				false);
		}
	}

	TryPlayActorSequencePlayer(NativeBpSkillSequence.Get());
}

void ACPlayableCharacter::End_SkillCam()
{
	if (false == IsLocallyControlled())
		return;

	APlayerController* pc = Cast<APlayerController>(GetController());
	if (false == IsValid(pc))
		pc = UGameplayStatics::GetPlayerController(this, 0);

	if (IsValid(NativeBpMainCamChild))
	{
		AActor* camActor = NativeBpMainCamChild->GetChildActor();
		if (IsValid(camActor) && IsValid(pc))
		{
			pc->SetViewTargetWithBlend(
				camActor,
				0.2f,
				EViewTargetBlendFunction::VTBlend_EaseInOut,
				1.f,
				false);
		}
	}

	SetZooming(OriginZooming);
}

void ACPlayableCharacter::ReportNoise()
{
	UCharacterMovementComponent* cm = GetCharacterMovement();
	if (false == IsValid(cm) || false == IsValid(MovementComp))
		return;

	if (cm->MaxWalkSpeed <= MovementComp->GetWalkSpeed())
		return;

	UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), 1.f, this, 0.f, NAME_None);
}

void ACPlayableCharacter::SetFluidSim()
{
	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return;
	if (nullptr == FluidSimClass.Get())
		return;

	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(world, FluidSimClass, actors);
	if (actors.Num() <= 0)
		return;

	FluidSimFolowing = actors[0];
	OnFluidSimActorRegistered(FluidSimFolowing.Get());
}

void ACPlayableCharacter::OnFluidSimActorRegistered_Implementation(AActor* InFluidSimActor)
{
	(void)InFluidSimActor;
}

void ACPlayableCharacter::Tick_Fluid()
{
	// 유체 액터 위치는 서버 권위로 맞춘다.
	if (false == HasAuthority())
		return;

	if (false == IsValid(FluidSimFolowing))
		return;

	FluidSimFolowing->SetActorLocation(GetActorLocation(), false, nullptr, ETeleportType::None);
}