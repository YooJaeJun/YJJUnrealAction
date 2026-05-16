#include "CCommonCharacter.h"
#include "Commons/CPlayerController.h"
#include "Global.h"
#include "Components/CapsuleComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CMontagesComponent.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/CCharacterStatComponent.h"
#include "Weapons/CWeaponStructures.h"
#include "UMG/Public/Blueprint/WidgetLayoutLibrary.h"
#include "Components/WidgetComponent.h"
#include "Widgets/CUserWidget_Custom.h"
#include "Components/SceneComponent.h"
#include "Widgets/Enemies/CUserWidget_EnemyBar.h"
#include "Interfaces/CInterface_Interactable.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/CharacterMovementComponent.h"

ACCommonCharacter::ACCommonCharacter()
{
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetCullDistanceSquared(FMath::Square(15000.0f));

	YJJHelpers::CreateActorComponent<UCStateComponent>(this, &StateComp, "YJJStateComponent");
	YJJHelpers::CreateActorComponent<UCMovementComponent>(this, &MovementComp, "YJJMovementComponent");

	StateComponent = StateComp;
	MovingComponent = MovementComp;
	YJJHelpers::CreateActorComponent<UCMontagesComponent>(this, &MontagesComp, "YJJMontagesComponent");
	YJJHelpers::CreateActorComponent<UCCharacterInfoComponent>(this, &CharacterInfoComp, "YJJCharacterInfoComponent");
	YJJHelpers::CreateActorComponent<UCCharacterStatComponent>(this, &CharacterStatComp, "YJJCharacterStatComponent");
	YJJHelpers::CreateComponent<USceneComponent>(this, &InfoPoint, "InfoPoint", GetMesh());
	YJJHelpers::CreateComponent<UWidgetComponent>(this, &InfoWidgetComp, "InfoWidgetComp", InfoPoint);
	YJJHelpers::CreateComponent<USceneComponent>(this, &TargetingPoint, "TargetingPoint", GetMesh());
	YJJHelpers::CreateComponent<UWidgetComponent>(this, &TargetingWidgetComp, "TargetingWidgetComp", TargetingPoint);
	YJJHelpers::CreateComponent<UTextRenderComponent>(this, &StateTextComponent, "StateTextComponent", GetCapsuleComponent());
	// CWB_* 위젯이 예전 모듈명(YJJActionCpp) 부모를 가리키면 CDO 시점의 GetClass 가 전체 캐릭터 로드를 깨뜨린다.
	// 클래스는 BeginPlay 에서 LoadClass + 폴백으로 적용한다.

	CharacterStatComp->OnHpIsZero.AddUObject(this, &ACCommonCharacter::Dead);

	if (IsValid(TargetingWidgetComp))
	{
		TargetingWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
		TargetingWidgetComp->SetVisibility(false);
	}

	if (IsValid(InfoWidgetComp))
	{
		InfoWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
		InfoWidgetComp->SetVisibility(true);
	}
}

void ACCommonCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyEmbeddedWidgetClassesIfNeeded();

	CurController = GetController();

	YJJHelpers::GetAssetDynamic<USoundBase>(&LandSound,
		TEXT("/Script/Engine.SoundWave'/Game/Assets/Sounds/Action/Sway_2.Sway_2'"));

	if (IsValid(TargetingPoint))
		TargetingPoint->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Targeting");

	// BP BeginPlay: MovingComponent GetSprintSpeed 를 CharacterMovement.MaxWalkSpeed 에 반영.
	if (IsValid(MovementComp))
		MovementComp->SetSprintSpeed();
}

void ACCommonCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bTickLerpForTarget && IsValid(CurController))
	{
		const FRotator start = CurController->GetControlRotation();
		const FRotator target = TargetRotator;

		CurController->SetControlRotation(
			FMath::RInterpTo(start, target, GetWorld()->DeltaTimeSeconds, 5.0f));

		if (false == UKismetMathLibrary::EqualEqual_Vector2DVector2D(
			UWidgetLayoutLibrary::GetMousePositionOnPlatform(), GetMousePos(), 1.0f))
			bTickLerpForTarget = false;

		if (UKismetMathLibrary::EqualEqual_RotatorRotator(start, target, 1.0f))
			bTickLerpForTarget = false;
	}
}

void ACCommonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Interact", EInputEvent::IE_Pressed, this, &ACCommonCharacter::InputAction_Interact);
}

void ACCommonCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	CheckNull(StateComp);
	StateComp->SetIdleMode();
}

float ACCommonCharacter::TakeDamage(
	float DamageAmount, 
	FDamageEvent const& DamageEvent,
	AController* EventInstigator, 
	AActor* DamageCauser)
{
	const float damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Damage.Power = damage;
	Damage.Attacker = Cast<ACCommonCharacter>(EventInstigator->GetPawn());
	Damage.Causer = DamageCauser;

	// FDamageEvent는 UObject가 아니므로 커스텀 데미지 이벤트 구조체로 해석해 히트 데이터를 꺼낸다.
	Damage.Event = *(FActDamageEvent*)&DamageEvent;

	StateComp->SetHitMode(Damage.Event.HitData.AttackType);

	return damage;
}

void ACCommonCharacter::Rise()
{
	CheckNull(MontagesComp);
	MontagesComp->PlayRiseAnim();
}

void ACCommonCharacter::Land()
{
	CheckNull(MontagesComp);
	MontagesComp->PlayLandAnim();
}

void ACCommonCharacter::Hit()
{
	// 누적된 피격 정보를 스탯 컴포넌트에 반영한다.
	CharacterStatComp->Damage(Damage.Power);
	Damage.Power = 0;
}

void ACCommonCharacter::Dead()
{
	IsDead.Broadcast();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CheckNull(MontagesComp);
	MontagesComp->PlayDeadAnim();

	GetWorldTimerManager().SetTimer(
		DestroyDelay_TimerHandle,
		this,
		&ACCommonCharacter::OnDestroyDelayTimer,
		1.5f,
		false,
		1.5f);
}

void ACCommonCharacter::OnDestroyDelayTimer()
{
	Destroy();
}

void ACCommonCharacter::End_Hit()
{
}

void ACCommonCharacter::End_Dead()
{
	Destroy();
}

void ACCommonCharacter::End_Rise()
{
	CheckNull(StateComp);
	StateComp->SetIdleMode();
}

void ACCommonCharacter::RestoreColor()
{
	ChangeColor(this, OriginColor);

	GetWorld()->GetTimerManager().ClearTimer(RestoreColor_TimerHandle);
}

void ACCommonCharacter::SetMyCurController(const TWeakObjectPtr<AController> InController)
{
	CurController = InController.Get();
}

void ACCommonCharacter::SetTickLerp(FRotator InRotator)
{
	TargetRotator = InRotator;
}

void ACCommonCharacter::TogglebTickLerpForTarget()
{
	bTickLerpForTarget = !bTickLerpForTarget;
}

void ACCommonCharacter::SetMousePos(const FVector2D InPos)
{
	MousePos = InPos;
}

void ACCommonCharacter::SetInteractor(TObjectPtr<ACCommonCharacter> InCharacter)
{
	Interactor = InCharacter;
}

void ACCommonCharacter::ApplyRestoreStateFromPrevMode()
{
	if (false == IsValid(StateComp))
		return;

	const CEStateType prev = StateComp->GetPrevMode();

	switch (prev)
	{
	case CEStateType::Idle:
	case CEStateType::Rise:
	case CEStateType::Avoid:
	case CEStateType::Land:
		StateComp->SetIdleMode();
		break;

	case CEStateType::Fall:
		StateComp->SetFallMode();
		break;

	case CEStateType::Equip:
	case CEStateType::Act:
		if (StateComp->IsRealRiding())
			StateComp->SetRidingMode();
		else
			StateComp->SetIdleMode();
		break;

	case CEStateType::Riding:
		StateComp->SetRidingMode();
		break;

	default:
		break;
	}
}

void ACCommonCharacter::EndHitted_ApplyRestoreIfStillInHit()
{
	if (false == IsValid(StateComp))
		return;

	if (StateComp->IsHitNoneMode())
		return;

	ApplyRestoreStateFromPrevMode();
	StateComp->SetHitNoneMode();
}

void ACCommonCharacter::End_Hitted()
{
	EndHitted_ApplyRestoreIfStillInHit();
}

void ACCommonCharacter::TogglePlayerPossessLocal(const bool bEnable)
{
	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return;

	APlayerController* playerController = UGameplayStatics::GetPlayerController(world, 0);
	if (false == IsValid(playerController))
		return;

	if (bEnable)
		playerController->Possess(this);
	else
		playerController->UnPossess();
}

CESpeedType ACCommonCharacter::GetCurrentSpeedType(const float Tolerance) const
{
	if (false == IsValid(MovementComp))
		return CESpeedType::Walk;

	const UCharacterMovementComponent* charMove = GetCharacterMovement();
	if (false == IsValid(charMove))
		return CESpeedType::Walk;

	const float currentSpeed = charMove->MaxWalkSpeed;

	for (uint8 index = 0; index < static_cast<uint8>(CESpeedType::Max); index++)
	{
		if (FMath::IsNearlyEqual(currentSpeed, MovementComp->Speeds[index], Tolerance))
			return static_cast<CESpeedType>(index);
	}

	return CESpeedType::Walk;
}

void ACCommonCharacter::LaunchBack()
{
	FVector velocity = -GetActorForwardVector() * ConstLaunchingBack;
	velocity.Z = LaunchBackVerticalImpulse;
	LaunchCharacter(velocity, false, false);
}

void ACCommonCharacter::LaunchHit()
{
	const FVector velocity = GetActorForwardVector() * (-HitData.Launch);
	LaunchCharacter(velocity, false, false);
}

void ACCommonCharacter::SetRotation(AActor* TargetActor, const bool bTeleportPhysics)
{
	if (false == IsValid(TargetActor))
		return;

	const FVector selfLocation = GetActorLocation();
	const FVector targetLocation = TargetActor->GetActorLocation();
	const FRotator lookAtRotation = UKismetMathLibrary::FindLookAtRotation(selfLocation, targetLocation);
	const FRotator yawOnly(0.0f, lookAtRotation.Yaw, 0.0f);

	const ETeleportType teleport = bTeleportPhysics ? ETeleportType::TeleportPhysics : ETeleportType::None;
	SetActorRotation(yawOnly, teleport);
}

void ACCommonCharacter::PlayParticle()
{
	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return;

	HitData.PlayEffect(world, GetActorLocation(), GetActorRotation());
}

void ACCommonCharacter::PlayHitSound()
{
	HitData.PlaySoundWave(this);
}

void ACCommonCharacter::PlaySound()
{
	PlayHitSound();
}

void ACCommonCharacter::RenderStateText()
{
	if (false == IsValid(StateTextComponent))
		return;

	if (false == StateTextComponent->IsVisible())
		StateTextComponent->SetVisibility(true, false);
}

void ACCommonCharacter::DontRenderStateText()
{
	if (false == IsValid(StateTextComponent))
		return;

	if (StateTextComponent->IsVisible())
		StateTextComponent->SetVisibility(false, false);
}

void ACCommonCharacter::BilboardStateText()
{
	if (false == IsValid(StateTextComponent))
		return;

	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return;

	APlayerCameraManager* cameraManager = UGameplayStatics::GetPlayerCameraManager(world, 0);
	if (false == IsValid(cameraManager))
		return;

	const FVector toCamera = cameraManager->GetCameraLocation() - StateTextComponent->GetComponentLocation();
	if (toCamera.IsNearlyZero())
		return;

	const FRotator billboardRotation = UKismetMathLibrary::MakeRotFromX(toCamera);
	StateTextComponent->SetWorldRotation(billboardRotation);
}

void ACCommonCharacter::TogglePossess_Implementation(const bool InEnable)
{
	TogglePlayerPossessLocal(InEnable);
}

void ACCommonCharacter::HoveredEquipMenu_Implementation(const FString& InName)
{
}

void ACCommonCharacter::UnHoveredEquipMenu_Implementation(const FString& InName)
{
}

void ACCommonCharacter::ClickedEquipMenu_Implementation(const FString& InName)
{
}

void ACCommonCharacter::HoveredMagicMenu_Implementation(const FString& InName)
{
}

void ACCommonCharacter::UnHoveredMagicMenu_Implementation(const FString& InName)
{
}

void ACCommonCharacter::ClickedMagicMenu_Implementation(const FString& InName)
{
}

void ACCommonCharacter::Footstep_Implementation()
{
}

void ACCommonCharacter::Rewarded_Implementation()
{
}

void ACCommonCharacter::Damaged_Implementation(float DamageAmount)
{
	// TakeDamage 경로와 별개로 애님·이펙트만 쏘는 레거시 이벤트일 수 있다.
	(void)DamageAmount;
}

void ACCommonCharacter::StartInteraction_Implementation(AActor* InteractionTarget)
{
	CurInteractingActor = InteractionTarget;
}

void ACCommonCharacter::EndInteraction_Implementation()
{
	CurInteractingActor = nullptr;
}

int32 ACCommonCharacter::GetAction_Implementation()
{
	return 0;
}

void ACCommonCharacter::SetIdle()
{
	if (IsValid(StateComp))
		StateComp->SetIdleMode();
}

void ACCommonCharacter::OnEquipMenu_Implementation()
{
}

void ACCommonCharacter::OffEquipMenu_Implementation()
{
}

void ACCommonCharacter::OnMagicMenu_Implementation()
{
}

void ACCommonCharacter::OffMagicMenu_Implementation()
{
}

void ACCommonCharacter::CanMove(const double InAxis, bool& OutCanMove) const
{
	OutCanMove = false;
	if (false == IsValid(MovementComp))
		return;

	// BP_Player::CanMove — |InAxis|>0.5 일 때만 MovingComponent::Is Can Move.
	const float axis = static_cast<float>(InAxis);
	if (FMath::Abs(axis) <= 0.5f)
		return;

	OutCanMove = MovementComp->IsCanMove();
}

void ACCommonCharacter::InputAction_Interact()
{
	OnInteract.Broadcast();

	AController* controller = GetController();
	if (IsValid(controller))
	{
		FVector viewLocation;
		FRotator viewRotation;
		controller->GetPlayerViewPoint(viewLocation, viewRotation);

		const FVector traceStart = viewLocation;
		const FVector traceEnd = traceStart + viewRotation.Vector() * 350.0f;

		// 마운트 전용 Overlap 상호작용보다 먼저, 시야에 잡힌 일반 Interactable을 처리한다.
		FHitResult hitResult;
		FCollisionQueryParams queryParams(SCENE_QUERY_STAT(InteractTrace), false, this);
		const bool bHit = GetWorld()->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, ECC_Visibility, queryParams);
		if (bHit && IsValid(hitResult.GetActor()))
		{
			ICInterface_Interactable* interactable = Cast<ICInterface_Interactable>(hitResult.GetActor());
			if (nullptr != interactable)
			{
				interactable->Interact(this);

				return;
			}
		}
	}

	if (IsValid(Interactor))
	{
		if (OnMount.IsBound())
			OnMount.Broadcast(this);
	}
	else
	{
		if (OnUnmount.IsBound())
			OnUnmount.Broadcast();
	}
}

void ACCommonCharacter::ApplyEmbeddedWidgetClassesIfNeeded()
{
	if (IsValid(InfoPoint))
		InfoPoint->SetWorldLocation(FVector(0, 500, 500));

	if (IsValid(InfoWidgetComp))
	{
		if (false == IsValid(InfoWidget))
		{
			UClass* enemyBarClass = LoadClass<UCUserWidget_EnemyBar>(
				nullptr, TEXT("/Game/Widgets/Enemy/CWB_Enemy_HpBar_Guage.CWB_Enemy_HpBar_Guage_C"));
			if (false == IsValid(enemyBarClass))
				enemyBarClass = LoadClass<UCUserWidget_EnemyBar>(
					nullptr, TEXT("/Game/Widgets/Enemy/WB_Enemy_HpBar_Guage.WB_Enemy_HpBar_Guage_C"));
			if (IsValid(enemyBarClass))
				InfoWidget = enemyBarClass;
			else
				CLog::Log(TEXT("[UI] 적 HP 바 위젯 클래스 로드 실패 — CWB_Enemy_HpBar_Guage 또는 WB_Enemy_HpBar_Guage 에셋·부모(UCUserWidget_EnemyBar) 확인"));
		}
		if (IsValid(InfoWidget))
			InfoWidgetComp->SetWidgetClass(InfoWidget);
	}

	if (IsValid(TargetingWidgetComp))
	{
		if (false == IsValid(TargetingWidget))
		{
			UClass* targetingClass = LoadClass<UCUserWidget_Custom>(
				nullptr, TEXT("/Game/Widgets/Interaction/CWB_Targeting.CWB_Targeting_C"));
			if (false == IsValid(targetingClass))
				targetingClass = LoadClass<UCUserWidget_Custom>(
					nullptr, TEXT("/Game/Widgets/Enemy/WB_Targeting.WB_Targeting_C"));
			if (IsValid(targetingClass))
				TargetingWidget = targetingClass;
			else
				CLog::Log(TEXT("[UI] 타겟팅 위젯 클래스 로드 실패 — CWB_Targeting 부모를 YJJActionCppUE5 UCUserWidget_Custom 으로 Reparent 하거나 WB_Targeting 사용"));
		}
		if (IsValid(TargetingWidget))
			TargetingWidgetComp->SetWidgetClass(TargetingWidget);
	}
}

UCUserWidget_HUD* ACCommonCharacter::EnsureHUDWidget()
{
	if (false == IsLocallyControlled())
		return nullptr;

	AController* controller = GetController();
	ACPlayerController* yjjPc = Cast<ACPlayerController>(controller);
	if (false == IsValid(yjjPc))
		return nullptr;

	return yjjPc->EnsureHUD();
}

UCUserWidget_HUD* ACCommonCharacter::GetPlayerHUDWidget() const
{
	return const_cast<ACCommonCharacter*>(this)->EnsureHUDWidget();
}
