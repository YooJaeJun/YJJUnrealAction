#include "Characters/Animals/CAnimal.h"

#include "Global.h"
#include "Characters/AI/CAIController_Animal.h"
#include "Animation/AnimMontage.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Blueprint/UserWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CWeaponComponent.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/CGameUIComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CCamComponent.h"
#include "Components/CPatrolComponent.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/CRidingComponent.h"
#include "Components/CMontagesComponent.h"
#include "Components/CTargetingComponent.h"
#include "Components/CStateComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/RotationMatrix.h"

namespace
{
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
		} payload{ InCur, InMax };
		Widget->ProcessEvent(fn, &payload);
	}
}

ACAnimal::ACAnimal()
{
	const TObjectPtr<USkeletalMeshComponent> mesh = GetMesh();

	// 레거시 BP_Animal SCS 변수명과 같은 UPROPERTY 식별자를 쓰면 스켈 병합 시 ObjectProperty 중복 또는
	// 다른 클래스 치환 assert 가 난다. 멤버 명 접두(DisplayName)·서브오브젝트 FName(YJJ…) 로 분리한다.
	YJJHelpers::CreateComponent<USceneComponent>(this, &MountLeftPoint, TEXT("MountLeft"), mesh);
	YJJHelpers::CreateComponent<USceneComponent>(this, &MountRightPoint, TEXT("MountRight"), mesh);
	YJJHelpers::CreateComponent<USceneComponent>(this, &MountBackPoint, TEXT("MountBack"), mesh);
	YJJHelpers::CreateComponent<USceneComponent>(this, &AnimalRiderPoint, TEXT("YJJAnimalRiderPoint"), mesh);
	YJJHelpers::CreateComponent<USceneComponent>(this, &UnmountPoint, TEXT("Unmount"), mesh);
	YJJHelpers::CreateComponent<UBoxComponent>(this, &InteractionCollision, TEXT("InterationCollision"), mesh);
	YJJHelpers::CreateComponent<USceneComponent>(this, &AnimalEyePoint, TEXT("YJJAnimalEyePoint"), mesh);

	YJJHelpers::CreateComponent<USpringArmComponent>(this, &AnimalSpringArm, TEXT("YJJAnimalSpringArm"), AnimalRiderPoint);
	YJJHelpers::CreateComponent<UCameraComponent>(this, &AnimalViewCamera, TEXT("YJJAnimalCamera"), AnimalSpringArm);

	YJJHelpers::CreateComponent<USceneComponent>(this, &HpBarSceneRoot, TEXT("Scene"), GetCapsuleComponent());
	YJJHelpers::CreateComponent<UWidgetComponent>(this, &HpBarWidgetComp, TEXT("HpBarWidget"), HpBarSceneRoot);

	YJJHelpers::CreateActorComponent<UCCamComponent>(this, &CamComp, "CamComponent");
	YJJHelpers::CreateActorComponent<UCGameUIComponent>(this, &GameUIComp, "GameUIComponent");
	YJJHelpers::CreateActorComponent<UCPatrolComponent>(this, &PatrolComp, "YJJAnimalPatrol");
	YJJHelpers::CreateActorComponent<UCRidingComponent>(this, &AnimalRidingComponent, TEXT("YJJAnimalRiding"));
	YJJHelpers::CreateActorComponent<UCWeaponComponent>(this, &WeaponComp, "WeaponComponent");
	YJJHelpers::CreateActorComponent<UCTargetingComponent>(this, &TargetingComp, "TargetingComponent");

	if (IsValid(StateComp))
	{
		StateComp->OnStateTypeChanged.AddUniqueDynamic(this, &ACAnimal::OnMountedAnimalStateTypeChanged);
		StateComp->OnHitStateTypeChanged.AddUniqueDynamic(this, &ACAnimal::OnMountedAnimalHitStateTypeChanged);
	}

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

	// LandingSound / LandEffect / CBP_Eye / AnimalWeapon 블프는 ctor 에서 동기 로드하면 BP_Animal↔AnimalWeapon 등과 순환·AsyncLoading2 Phase2 에서 깨진다 —
	// `AnimalEnsureLandingAndDeferredBlueprintAssetsLoadedAfterCommonBeginPlay()` 에서만 보충한다.

	if (IsValid(AnimalSpringArm))
	{
		AnimalSpringArm->bDoCollisionTest = false;
		AnimalSpringArm->SetRelativeLocation(FVector(0, 3, 100));
		AnimalSpringArm->SetRelativeRotation(FRotator(-5, 90, 0));
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

	AIControllerClass = ACAIController_Animal::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

UWidgetComponent* ACAnimal::GetAnimalHpBarWidgetComponent() const
{
	return HpBarWidgetComp.Get();
}

void ACAnimal::BeginPlay()
{
	Super::BeginPlay();

	AnimalEnsureLandingAndDeferredBlueprintAssetsLoadedAfterCommonBeginPlay();

	// 스탯 기반 Hp 미러 및 UI 초기 패스.
	SetHp();
	SetHpUI();

	SpawnAnimalWeaponFromClassIfConfigured();
	if (IsValid(HpBarWidgetComp) && AnimalHpBarWidgetClass != nullptr)
		HpBarWidgetComp->SetWidgetClass(AnimalHpBarWidgetClass);

	if (IsValid(AnimalRidingComponent) && IsValid(InteractionCollision))
	{
		InteractionCollision->OnComponentBeginOverlap.AddDynamic(
			AnimalRidingComponent.Get(), &UCRidingComponent::MountInteraction_OnBeginOverlap);
		InteractionCollision->OnComponentEndOverlap.AddDynamic(
			AnimalRidingComponent.Get(), &UCRidingComponent::MountInteraction_OnEndOverlap);
	}

	if (IsValid(AnimalRiderPoint))
		AnimalRiderPoint->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "Rider");

	if (IsValid(AnimalEyePoint))
	{
		AnimalEyePoint->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "EyeEffect");

		if (IsValid(EyeClass))
		{
			FActorSpawnParameters params;
			params.Owner = Cast<AActor>(this);

			Eye = GetWorld()->SpawnActor<AActor>(EyeClass,
				AnimalEyePoint->GetComponentLocation(), AnimalEyePoint->GetComponentRotation(), params);

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

	// HpBarWidgetComp 클래스 지정 후 위젯 캐시를 다시 채운다.
	SetHpUI();
}

void ACAnimal::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UCMovementComponent* const resolvedMovement = EnsureMovementComp();
	if (IsValid(resolvedMovement))
	{
		PlayerInputComponent->BindAxis("MoveForward", resolvedMovement, &UCMovementComponent::InputAxis_MoveForward);
		PlayerInputComponent->BindAxis("MoveRight", resolvedMovement, &UCMovementComponent::InputAxis_MoveRight);

		PlayerInputComponent->BindAction("Walk", IE_Pressed, resolvedMovement, &UCMovementComponent::InputAction_Walk);
		PlayerInputComponent->BindAction("Walk", IE_Released, resolvedMovement, &UCMovementComponent::InputAction_Run);
		PlayerInputComponent->BindAction("Jump", IE_Pressed, resolvedMovement, &UCMovementComponent::InputAction_Jump);
	}
	else if (IsLocallyControlled())
	{
		CLog::Log(FString::Printf(
			TEXT("[입력 바인딩] MovementComp 없음 — 이동 축/Walk/Jump 미바인딩. BP 레거시 컴포넌트 중복 여부 확인. Actor=%s"),
			*GetNameSafe(this)));
	}

	PlayerInputComponent->BindAxis("HorizontalLook", CamComp.Get(), &UCCamComponent::InputAxis_HorizontalLook);
	PlayerInputComponent->BindAxis("VerticalLook", CamComp.Get(), &UCCamComponent::InputAxis_VerticalLook);
	PlayerInputComponent->BindAxis("Zoom", AnimalRidingComponent.Get(), &UCRidingComponent::Input_Zoom);
	PlayerInputComponent->BindAction("Targeting", IE_Pressed, AnimalRidingComponent.Get(), &UCRidingComponent::TargetingInput);
	PlayerInputComponent->BindAction("Menu", IE_Pressed, AnimalRidingComponent.Get(), &UCRidingComponent::Ride_Input_Menu);
	PlayerInputComponent->BindAction("Menu", IE_Released, AnimalRidingComponent.Get(), &UCRidingComponent::Ride_Input_MenuHide);
	PlayerInputComponent->BindAction("MagicMenu", IE_Pressed, AnimalRidingComponent.Get(), &UCRidingComponent::Ride_Input_MagicMenu);
	PlayerInputComponent->BindAction("MagicMenu", IE_Released, AnimalRidingComponent.Get(), &UCRidingComponent::Ride_Input_MagicMenuHide);
	PlayerInputComponent->BindAction("Action", IE_Pressed, AnimalRidingComponent.Get(), &UCRidingComponent::Ride_Input_Action);
	PlayerInputComponent->BindAction("SubWeapon_Action", IE_Pressed, AnimalRidingComponent.Get(), &UCRidingComponent::Ride_Input_SubWeaponPressed);
	PlayerInputComponent->BindAction("SubWeapon_Action", IE_Released, AnimalRidingComponent.Get(), &UCRidingComponent::Ride_Input_SubWeaponReleased);
	PlayerInputComponent->BindAction("Skill_1", IE_Pressed, AnimalRidingComponent.Get(), &UCRidingComponent::Ride_Input_Skill1Pressed);
	PlayerInputComponent->BindAction("Skill_2", IE_Pressed, AnimalRidingComponent.Get(), &UCRidingComponent::Ride_Input_Skill2Pressed);
	PlayerInputComponent->BindAction("Magic", IE_Pressed, AnimalRidingComponent.Get(), &UCRidingComponent::Ride_Input_Magic);
}

void ACAnimal::InputAction_Interact()
{
	if (GetbRiding() && IsValid(AnimalRidingComponent.Get()) && AnimalRidingComponent->GetRider().IsValid())
	{
		AnimalRidingComponent->EndInteraction();
		return;
	}

	ACCommonCharacter::InputAction_Interact();
}

void ACAnimal::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (IsValid(MovementComp))
		MovementComp->SetGravity(1.f);

	PlayLandMontageIfAny();

	UGameplayStatics::PlaySoundAtLocation(this, LandSound, GetActorLocation());

	FTransform landEffectTransform = GetActorTransform();
	landEffectTransform.SetScale3D(landEffectTransform.GetScale3D() * LandEffectScaleFactor);

	YJJHelpers::PlayEffect(GetWorld(), LandEffect, landEffectTransform);
}

void ACAnimal::Hit()
{
	CheckNull(MontagesComp);
	MontagesComp->PlayAvoidAnim();

	Super::Hit();

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

void ACAnimal::OnMountedAnimalStateTypeChanged(const CEStateType InPrevType, const CEStateType InNewType)
{
	(void)InPrevType;

	switch (InNewType)
	{
	case CEStateType::Land:
		Land();
		break;
	case CEStateType::Dead:
		Dead();
		break;
	default:
		break;
	}
}

void ACAnimal::OnMountedAnimalHitStateTypeChanged(const CEHitType InPrevType, const CEHitType InNewType)
{
	(void)InPrevType;
	(void)InNewType;

	Hit();
}

void ACAnimal::SetZoomMinRange(const float InMinRange) const
{
	CheckNull(CamComp);
	CamComp->ZoomData.MinRange = InMinRange;
}

void ACAnimal::SetZoomMaxRange(const float InMaxRange) const
{
	CheckNull(CamComp);
	CamComp->ZoomData.MaxRange = InMaxRange;
}

TObjectPtr<USpringArmComponent> ACAnimal::GetSpringArm() const
{
	return AnimalSpringArm;
}

TObjectPtr<UCTargetingComponent> ACAnimal::GetTargetingComp() const
{
	return TargetingComp;
}

UBehaviorTree* ACAnimal::GetAnimalBehaviorTreeForController() const
{
	// CDO ctor 에서 BehaviorTree 동기 로드(BT_Animal_Run 등)하면 BB_Enemy·BP_Player 체인이 ACAnimal 과 순환 패키지로 AsyncLoading 교착이 날 수 있다.
	if (false == IsValid(BehaviorTree))
	{
		ACAnimal* const mutableAnimal = const_cast<ACAnimal*>(this);
		YJJHelpers::GetAssetDynamic<UBehaviorTree>(
			&mutableAnimal->BehaviorTree,
			FString(TEXT("/Script/AIModule.BehaviorTree'/Game/Character/Animals/BT_Animal_Run.BT_Animal_Run'")));
	}
	return BehaviorTree.Get();
}

void ACAnimal::AnimalEnsureLandingAndDeferredBlueprintAssetsLoadedAfterCommonBeginPlay()
{
	// 부모 BeginPlay 의 LandSound 할당 후에 동물 전용 에셋을 덮어쓴다(레거시 ctor 와 동일한 우선순위).
	YJJHelpers::GetAssetDynamic<USoundBase>(&LandSound,
		FString(TEXT("/Script/Engine.SoundCue'/Game/Assets/Sounds/Footsteps/Run/Stone/SC_Footstep_Stone_Run.SC_Footstep_Stone_Run'")));
	YJJHelpers::GetAssetDynamic<UFXSystemAsset>(&LandEffect,
		FString(TEXT("/Script/Niagara.NiagaraSystem'/Game/Assets/Effects/SuperheroFlight/VFX/Niagara/System/SuperheroLanding/NS_Superhero_Landing_Concrete.NS_Superhero_Landing_Concrete'")));

	if (nullptr == EyeClass)
	{
		YJJHelpers::GetClassDynamic<AActor>(&EyeClass,
			FString(TEXT("/Script/Engine.Blueprint'/Game/Character/Animals/CBP_Eye.CBP_Eye_C'")));
		if (nullptr == EyeClass)
		{
			CLog::Log(FString::Printf(TEXT("ACAnimal: CBP_Eye 로드 실패 — Eye 포인터 스폰 분기 무시 가능 [%s]"), *GetNameSafe(this)));
		}
	}

	if (true == bAnimalFillDefaultWeaponClassFromDiskWhenUnset && nullptr == AnimalWeaponClass)
	{
		YJJHelpers::GetClassDynamic<AActor>(&AnimalWeaponClass,
			FString(TEXT("/Script/Engine.Blueprint'/Game/Character/Animals/AnimalWeapon.AnimalWeapon_C'")));
		if (nullptr == AnimalWeaponClass)
		{
			CLog::Log(FString::Printf(TEXT("ACAnimal: AnimalWeapon 디스크 기본 블프 로드 실패 — 무기 미스폰 [%s]"), *GetNameSafe(this)));
		}
	}
}

void ACAnimal::SpawnAnimalWeaponFromClassIfConfigured()
{
	if (IsValid(AnimalWeapon))
		return;

	if (nullptr == AnimalWeaponClass)
		return;

	UWorld* const world = GetWorld();
	if (nullptr == world)
		return;

	FActorSpawnParameters params;
	params.Owner = this;

	AActor* const spawnedActor = world->SpawnActor<AActor>(AnimalWeaponClass, GetActorTransform(), params);
	if (IsValid(spawnedActor))
	{
		AnimalWeapon = spawnedActor;
		return;
	}

	UE_LOG(
		LogTemp,
		Error,
		TEXT("[ACAnimal] AnimalWeapon 스폰 실패 — 클래스/레벨 상태 확인 필요 (%s)."),
		*GetNameSafe(this));
}

void ACAnimal::SetHp()
{
	if (IsValid(CharacterStatComp))
	{
		Hp = static_cast<double>(CharacterStatComp->GetCurHp());
		MaxHp = static_cast<double>(CharacterStatComp->GetMaxHp());
	}
}

void ACAnimal::SetHpUI()
{
	UWidgetComponent* barComp = GetAnimalHpBarWidgetComponent();
	if (IsValid(barComp))
	{
		// BP_SetHpUI: WidgetComponent 의 UserWidget 을 HpBar_NPC 에 캐시 후 게이지 갱신.
		UUserWidget* userW = barComp->GetUserWidgetObject();
		if (IsValid(userW))
			HpBar_NPC = userW;

		barComp->SetVisibility(bOnUIInfo, false);
	}

	ApplyEnemyHpBarPercent();
}

void ACAnimal::UpdateHp_NPC()
{
	ApplyEnemyHpBarPercent();
}

void ACAnimal::ApplyEnemyHpBarPercent()
{
	UUserWidget* widget = Cast<UUserWidget>(HpBar_NPC.Get());
	if (false == IsValid(widget))
		return;

	const double pct = (MaxHp > static_cast<double>(KINDA_SMALL_NUMBER))
		? FMath::Clamp(Hp / MaxHp, 0.0, 1.0)
		: 0.0;

	UFunction* pctFn = widget->FindFunction(FName(TEXT("SetHpPercent")));
	if (pctFn != nullptr)
	{
		struct FSetHpPctPay
		{
			double HpPercent;
		} pay{ pct };

		widget->ProcessEvent(pctFn, &pay);
	}

	CallWidgetSetHpUi(widget, Hp, MaxHp);
}

void ACAnimal::RefreshAnimalHpBarWidgets()
{
	ApplyEnemyHpBarPercent();
}

void ACAnimal::SetDamage(const float InDamage, bool& OutHittedOrDead)
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

	SetHpUI();

	OnUpdateHp_Riding.Broadcast();

	OutHittedOrDead = Hp > 0.0;
}

void ACAnimal::PlayHitAnim()
{
	USkeletalMeshComponent* mesh = GetMesh();
	if (false == IsValid(mesh))
		return;

	if (HitData.Montage != nullptr)
	{
		PlayAnimMontage(HitData.Montage, HitData.PlayRate);
		return;
	}

	if (IsValid(HitAnim))
		PlayAnimMontage(HitAnim.Get(), 1.0f);
}

void ACAnimal::PlayHitUniqueSound()
{
	if (false == IsValid(HitUniqueSound))
		return;

	UGameplayStatics::PlaySoundAtLocation(this, HitUniqueSound.Get(), GetActorLocation());
}

void ACAnimal::SpawnBlood()
{
	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return;

	const FVector loc = HitPoint.IsNearlyZero() ? GetActorLocation() : HitPoint;

	if (BloodActorClass != nullptr)
	{
		const FTransform spawnTm(GetActorQuat(), loc, GetActorScale3D());
		FActorSpawnParameters sp;
		sp.Owner = this;
		sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		world->SpawnActor<AActor>(BloodActorClass, spawnTm, sp);
	}

	if (IsValid(BloodSound))
		UGameplayStatics::PlaySoundAtLocation(this, BloodSound, loc);

	// BP_SpawnBlood 에 없던 레거시 — Niagara 만 있던 동물은 그대로 재생.
	if (IsValid(BloodEffect))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			world,
			BloodEffect.Get(),
			loc,
			GetActorRotation(),
			FVector::OneVector,
			true,
			true,
			ENCPoolMethod::None,
			true);
	}
}

void ACAnimal::LevelUp()
{
	Level = Level + 1;
	Exp = FMath::Max(0.0, Exp - MaxExp);
}

void ACAnimal::LevelUpAnimal()
{
	LevelUp();
}

void ACAnimal::LoadPrevState()
{
	if (false == IsValid(StateComp))
		return;

	if (StateComp->IsDead())
		return;

	StateComp->SetIdle();
}

void ACAnimal::ApplyRewardedEvent(ACharacter* Invoker, const double InExp, const int32 BuffIndex)
{
	(void)Invoker;
	(void)BuffIndex;

	Exp += InExp;
	if (Exp > MaxExp && MaxExp > 0.0)
	{
		LevelUpAnimal();
		OnUpdateLevel_Riding.Broadcast();
	}
	OnUpdateExp_Riding.Broadcast();
}

void ACAnimal::TryGrantKillRewardToAttacker()
{
	if (false == IsValid(Attacker))
		return;

	UFunction* fn = Attacker->FindFunction(FName(TEXT("Rewarded")));
	if (fn == nullptr)
		return;

	struct FRewardPay
	{
		ACharacter* Invoker;
		double Exp;
		int32 BuffIndex;
	} pay{ Cast<ACharacter>(this), static_cast<double>(RewardExp), 0 };

	Attacker->ProcessEvent(fn, &pay);
}

void ACAnimal::InvokeHittedEffects()
{
	bool bAlive = false;
	SetDamage(0.0f, bAlive);

	if (bAlive)
	{
		if (IsValid(StateComp))
			StateComp->SetHitCommonMode();

		PlayHitAnim();
		PlayParticle();
		PlaySound();
		PlayHitStop();
		PlayCameraShake();

		PlayHitUniqueSound();
		SpawnBlood();
	}
	else
	{
		if (IsValid(StateComp))
			StateComp->SetDeadMode();
	}
}

void ACAnimal::Dead()
{
	if (bAnimalDeathSequenceStarted)
		return;

	bAnimalDeathSequenceStarted = true;

	OnIsDead.Broadcast();

	if (IsValid(StateComp))
		StateComp->SetDeadMode();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	USkeletalMeshComponent* mesh = GetMesh();
	if (IsValid(DeadAnim) && IsValid(mesh))
		PlayAnimMontage(DeadAnim.Get());
	else if (IsValid(MontagesComp))
		MontagesComp->PlayDeadAnim();

	TryGrantKillRewardToAttacker();

	if (IsValid(DeadSound))
		UGameplayStatics::PlaySoundAtLocation(this, DeadSound.Get(), GetActorLocation());

	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return;

	FTimerDelegate delayAfterReward;
	delayAfterReward.BindUObject(this, &ACAnimal::OnAnimalDeathAfterRewardDelay);
	world->GetTimerManager().SetTimer(AnimalDeathTimer_RewardDelay, delayAfterReward, 0.2f, false);
}

void ACAnimal::OnAnimalDeathAfterRewardDelay()
{
	End_Dead();
}

void ACAnimal::End_Dead()
{
	UWorld* world = GetWorld();
	if (false == IsValid(world))
	{
		Destroy();
		return;
	}

	FTimerDelegate soulFx;
	soulFx.BindUObject(this, &ACAnimal::OnAnimalDeathSpawnSoulAndDestroy);
	world->GetTimerManager().SetTimer(AnimalDeathTimer_SoulFx, soulFx, 1.5f, false);
}

void ACAnimal::OnAnimalDeathSpawnSoulAndDestroy()
{
	UWorld* world = GetWorld();
	if (IsValid(DeathSoulEmitterTemplate) && IsValid(world))
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			world,
			DeathSoulEmitterTemplate.Get(),
			GetActorLocation(),
			FRotator::ZeroRotator,
			FVector::OneVector,
			true,
			EPSCPoolMethod::None,
			true);
	}

	Destroy();
}

void ACAnimal::FootstepAt(const bool bLeftOrRight, const EPhysicalSurface SurfaceType, const FVector StepLocation)
{
	(void)bLeftOrRight;

	const int32 surfaceIndex = static_cast<int32>(SurfaceType);

	if (FootstepSounds.IsValidIndex(surfaceIndex) && IsValid(FootstepSounds[surfaceIndex]))
		UGameplayStatics::PlaySoundAtLocation(this, FootstepSounds[surfaceIndex], StepLocation);

	if (FootstepEffects.IsValidIndex(surfaceIndex) && IsValid(FootstepEffects[surfaceIndex]))
	{
		UWorld* world = GetWorld();
		if (IsValid(world))
		{
			const FVector forward = GetActorForwardVector();
			const FRotator rot = UKismetMathLibrary::MakeRotFromX(forward);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				world,
				FootstepEffects[surfaceIndex],
				StepLocation,
				rot,
				FVector(0.5f, 0.5f, 0.5f),
				true,
				true,
				ENCPoolMethod::None,
				true);
		}
	}
}

void ACAnimal::LandAt(const EPhysicalSurface SurfaceType, const FVector StepLocation)
{
	const int32 surfaceIndex = static_cast<int32>(SurfaceType);

	if (FootstepSounds.IsValidIndex(surfaceIndex) && IsValid(FootstepSounds[surfaceIndex]))
		UGameplayStatics::PlaySoundAtLocation(this, FootstepSounds[surfaceIndex], StepLocation);

	if (LandEffects.IsValidIndex(surfaceIndex) && IsValid(LandEffects[surfaceIndex]))
	{
		UWorld* world = GetWorld();
		if (IsValid(world))
		{
			const FVector forward = GetActorForwardVector();
			const FRotator rot = UKismetMathLibrary::MakeRotFromX(forward);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				world,
				LandEffects[surfaceIndex],
				StepLocation,
				rot,
				FVector::OneVector,
				true,
				true,
				ENCPoolMethod::None,
				true);
		}
	}
}

void ACAnimal::Footstep_Implementation(bool bLeftFoot, EPhysicalSurface SurfaceType, FVector HitLocation)
{
	FootstepAt(bLeftFoot, SurfaceType, HitLocation);
}

void ACAnimal::ToggleIK_Implementation()
{
	// ICInterface_IK 의 알파를 토글 — 탑승 시 AnimalRidingComponent 가 별도 값을 줄 수 있다.
	const float cur = GetLegIKAlpha();
	if (cur > 0.01f)
		SetLegIKAlpha(0.0f);
	else
		SetLegIKAlpha(0.3f);
}

void ACAnimal::GetControlDirection(FVector& OutForward, FVector& OutRight) const
{
	const FRotator controlRot = GetControlRotation();
	const FRotator yawOnly(0.0f, controlRot.Yaw, 0.0f);
	OutForward = yawOnly.Vector();
	OutRight = FRotationMatrix(yawOnly).GetUnitAxis(EAxis::Y);
}

void ACAnimal::GetAnimalDesiredMovement_Implementation(FVector& OutMovement)
{
	float axisForward = 0.0f;
	float axisRight = 0.0f;

	const APlayerController* pc = Cast<APlayerController>(GetController());
	if (IsValid(pc))
	{
		axisForward = pc->GetInputAxisValue(FName(TEXT("MoveForward")));
		axisRight = pc->GetInputAxisValue(FName(TEXT("MoveRight")));
	}

	const FRotator controlRot = GetControlRotation();
	const FRotator yawOnly(0.0f, controlRot.Yaw, 0.0f);
	const FVector forward = yawOnly.Vector();
	const FVector right = FRotationMatrix(yawOnly).GetUnitAxis(EAxis::Y);

	const FVector combined = forward * axisForward + right * axisRight;

	const CESpeedType speedType = GetCurrentSpeedType(1.0f);
	float speedMul = 1.0f;
	switch (speedType)
	{
	case CESpeedType::Run:
		speedMul = 1.7f;
		break;
	case CESpeedType::Sprint:
		speedMul = 2.5f;
		break;
	case CESpeedType::Walk:
	default:
		speedMul = 1.0f;
		break;
	}

	OutMovement = combined * speedMul;
}

void ACAnimal::SetFootLocation_Implementation(bool bLeftFoot, FVector WorldLocation)
{
	// 구 BP 는 AnimBP·풋 IK 타깃 전달용 — 네이티브 기본은 노옵(블루프린트에서 확장).
	(void)bLeftFoot;
	(void)WorldLocation;
}

void ACAnimal::PlayHitStop()
{
	if (FMath::IsNearlyZero(HitData.HitStop))
		return;

	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return;

	RestoreTimeDilation();

	DilationActors.Reset();

	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(world, AActor::StaticClass(), actors);

	for (int32 i = 0; i < actors.Num(); i++)
	{
		AActor* actor = actors[i];
		if (false == IsValid(actor))
			continue;

		bool bShouldDilate = false;
		const APawn* asPawn = Cast<APawn>(actor);
		if (IsValid(asPawn))
			bShouldDilate = true;
		else
		{
			UMeshComponent* mesh = actor->FindComponentByClass<UMeshComponent>();
			if (IsValid(mesh) && mesh->Mobility == EComponentMobility::Movable)
				bShouldDilate = true;
		}

		if (false == bShouldDilate)
			continue;

		DilationActors.Add(actor);
		actor->CustomTimeDilation = 0.001f;
	}

	world->GetTimerManager().ClearTimer(HitStopRestoreTimer);

	FTimerDelegate restoreDel;
	restoreDel.BindUObject(this, &ACAnimal::RestoreTimeDilation);
	world->GetTimerManager().SetTimer(HitStopRestoreTimer, restoreDel, HitData.HitStop, false);
}

void ACAnimal::RestoreTimeDilation()
{
	for (int32 i = 0; i < DilationActors.Num(); i++)
	{
		AActor* actor = DilationActors[i].Get();
		if (IsValid(actor))
			actor->CustomTimeDilation = 1.0f;
	}

	DilationActors.Reset();

	UWorld* world = GetWorld();
	if (IsValid(world))
		world->GetTimerManager().ClearTimer(HitStopRestoreTimer);
}

void ACAnimal::PlayCameraShake()
{
	if (HitData.ShakeClass == nullptr)
		return;

	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return;

	if (world->GetNetMode() == NM_DedicatedServer)
		return;

	ACCommonCharacter* atk = Attacker.Get();
	if (false == IsValid(atk))
		return;

	if (false == atk->IsPlayerControlled())
		return;

	if (false == atk->IsLocallyControlled())
		return;

	APlayerController* pc = Cast<APlayerController>(atk->GetController());
	if (false == IsValid(pc))
		return;

	APlayerCameraManager* cam = pc->PlayerCameraManager;
	if (false == IsValid(cam))
		return;

	cam->StartCameraShake(HitData.ShakeClass, 1.0f, ECameraShakePlaySpace::CameraLocal);
}

float ACAnimal::PlayLandMontageIfAny()
{
	if (false == IsValid(LandAnim) || false == IsValid(GetMesh()))
		return 0.0f;

	return PlayAnimMontage(LandAnim.Get());
}
