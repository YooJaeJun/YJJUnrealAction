#include "Characters/Animals/Dragon/CDragon.h"
#include "Characters/Animals/Dragon/Weapon/CDragonWeapon.h"
#include "Commons/CEnums.h"
#include "Global.h"
#include "Animation/AnimMontage.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraShakeBase.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CFlyComponent.h"
#include "Components/CGameUIComponent.h"
#include "Components/CRidingComponent.h"
#include "Components/CSystemMessageComponent.h"
#include "Components/CTargetingComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Boss/CUserWidget_BossGroggyBar.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"

namespace
{
	constexpr float DragonVerticalFlyBaselinePositive = 20.0f;
	constexpr float DragonVerticalFlyBaselineNegative = 50.0f;

	void CallBossWidgetHpPercent(UUserWidget* Widget, const double HpPercent)
	{
		if (false == IsValid(Widget))
			return;

		UFunction* const fn = Widget->FindFunction(FName(TEXT("SetHpPercent")));
		if (nullptr == fn)
			return;

		struct FPay
		{
			double HpPercent;
		} pay{ HpPercent };

		Widget->ProcessEvent(fn, &pay);
	}

	void CallBossWidgetSetHpUi(UUserWidget* Widget, const double InCur, const double InMax)
	{
		if (false == IsValid(Widget))
			return;

		UFunction* const fn = Widget->FindFunction(FName(TEXT("SetHPUI")));
		if (nullptr == fn)
			return;

		struct FPay
		{
			double InCur;
			double InMax;
		} pay{ InCur, InMax };

		Widget->ProcessEvent(fn, &pay);
	}

	void InvokeBossGroggyBarRefresh(UUserWidget* Widget, const double CurGroggy, const double MaxGroggyVal)
	{
		if (false == IsValid(Widget))
			return;

		UFunction* fnSetUi = Widget->FindFunction(FName(TEXT("SetGroggyUI")));
		if (nullptr != fnSetUi)
		{
			struct FGroggyUiPay
			{
				double InCur;
				double InMax;
			} payload{ CurGroggy, MaxGroggyVal };

			Widget->ProcessEvent(fnSetUi, &payload);
			return;
		}

		UFunction* fnPct = Widget->FindFunction(FName(TEXT("SetGroggyPercent")));
		if (nullptr != fnPct)
		{
			const double pct = UCUserWidget_BossGroggyBar::ComputeGroggyRatio(CurGroggy, MaxGroggyVal);

			struct FPctPay
			{
				double GroggyPercent;
			} pay{ pct };

			Widget->ProcessEvent(fnPct, &pay);
			return;
		}

		UFunction* fnUpd = Widget->FindFunction(FName(TEXT("UpdateGroggy")));
		if (nullptr != fnUpd)
		{
			Widget->ProcessEvent(fnUpd, nullptr);
		}
	}

	// BP Moving graph tuning defaults (legacy)
	constexpr float DragonMovingGraphStrafeUnitsPerPoll = 40.0f;
	constexpr float DragonMovingGraphAirRollClampDegrees = 60.0f;
	constexpr float DragonMovingGraphAirPitchClampDegrees = 60.0f;
	constexpr float DragonMovingGraphLevelingInterpSpeed = 2.0f;
	constexpr float DragonJumpBlueprintGravity = 0.05f;
}

ACDragon::ACDragon()
{
	YJJHelpers::CreateActorComponent<UCFlyComponent>(this, &FlyComp, "FlyComponent");

	YJJHelpers::CreateActorComponent<UCSystemMessageComponent>(this, &SystemMessageComp, "SystemMessageComponent");

	const TObjectPtr<USkeletalMeshComponent> mesh = GetMesh();
	YJJHelpers::CreateComponent<USceneComponent>(this, &Neck, TEXT("Neck"), mesh);

	// Do not spawn generic AnimalWeapon; dragon uses DragonWeapon (ACAnimal::BeginPlay would spawn AnimalWeapon otherwise).
	AnimalWeaponClass = nullptr;
	bAnimalFillDefaultWeaponClassFromDiskWhenUnset = false;
	AnimalWeapon = nullptr;
	DragonWeapon = nullptr;

	YJJHelpers::GetAsset<UNiagaraSystem>(&JumpEffect,
		TEXT("/Script/Niagara.NiagaraSystem'/Game/Assets/Effects/SuperheroFlight/VFX/Niagara/System/SuperheroLanding/NS_Superhero_Landing_Concrete.NS_Superhero_Landing_Concrete'"));

	YJJHelpers::GetAsset<UAnimMontage>(&GroggyAnim,
		TEXT("/Script/Engine.AnimMontage'/Game/Assets/Animals/Dragon/Animations/Blood_Dragon/In_Place/Dragon_Death_Anim_Montage.Dragon_Death_Anim_Montage'"));

	YJJHelpers::GetClass<AActor>(&DragonWeaponClass,
		TEXT("/Script/Engine.Blueprint'/Game/Character/Animals/Dragon/Weapon/DragonWeapon.DragonWeapon_C'"));

	// Legacy BP Footstep/Land default asset path (matches DefaultObject).
	YJJHelpers::GetClass<UCameraShakeBase>(&LandCameraShakeClass,
		TEXT("/Script/Engine.Blueprint'/Game/Character/Player/CS_NotEnoughState.CS_NotEnoughState_C'"));

	YJJHelpers::GetAsset<UParticleSystem>(&DeathSoulEmitterTemplate,
		TEXT("/Script/Engine.ParticleSystem'/Game/Assets/Effects/TrailPack/Particles/Soul/Aura/P_SoulAura.P_SoulAura'"));

	DragonSpawnAnnouncementMessage =
		FText::FromString(TEXT("Ragnarok has descended."));
}

void ACDragon::BeginPlay()
{
	BindJumpEffectIntoLandEffectFallback();

	if (nullptr != EyeEffect)
	{
		EyeClass = EyeEffect;
	}

	Super::BeginPlay();

	ApplyNeckAttachToDragonMeshSocketFireGround();

	SpawnDragonWeaponFromClassIfConfigured();
	SyncFlyVerticalStrengthFromUpFactor();

	SetZoomMinRange(50.0f);
	SetZoomMaxRange(1000.0f);

	LandEffectScaleFactor = 2.0f;

	if (IsValid(CharacterStatComp))
		CharacterStatComp->SetAttackRange(1200.0f);
}

void ACDragon::BindJumpEffectIntoLandEffectFallback()
{
	if (false == IsValid(JumpEffect))
		return;

	// Niagara jump FX doubles as landing FX asset for FlyComp and LandEffect slot.
	LandEffect = JumpEffect.Get();
}

void ACDragon::SpawnDragonWeaponFromClassIfConfigured()
{
	if (IsValid(DragonWeapon))
		return;

	if (nullptr == DragonWeaponClass)
		return;

	UWorld* const world = GetWorld();
	if (nullptr == world)
		return;

	FActorSpawnParameters params;
	params.Owner = this;

	AActor* const spawnedActor = world->SpawnActor<AActor>(DragonWeaponClass, GetActorTransform(), params);
	if (IsValid(spawnedActor))
	{
		DragonWeapon = spawnedActor;
		return;
	}

	UE_LOG(
		LogTemp,
		Error,
		TEXT("[ACDragon] DragonWeapon spawn failed - check DragonWeaponClass / map (%s)."),
		*GetNameSafe(this));
}

void ACDragon::SyncFlyVerticalStrengthFromUpFactor()
{
	if (false == IsValid(FlyComp))
		return;

	// UpFactor==0 uses internal 20/50 baseline; magnitude scales by abs(UpFactor), sign comes from Dragon_InputAxis_FlyUp axis.
	const float multiplier = FMath::IsNearlyZero(UpFactor) ? 1.0f : FMath::Abs(UpFactor);

	FlyComp->SetFlyVerticalStrengths(DragonVerticalFlyBaselinePositive * multiplier, DragonVerticalFlyBaselineNegative * multiplier);
}

void ACDragon::SetupCinematic_Implementation(const bool OnOff)
{
	if (nullptr == StateComp)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ACDragon] SetupCinematic: missing StateComp - %s."),
			*GetNameSafe(this));
		return;
	}

	if (OnOff)
	{
		StateComp->SetCinematic();
		SetGravity(0.0);

		if (IsValid(BossInfoUi))
			BossInfoUi->SetVisibility(ESlateVisibility::Collapsed);

		SetGroupIndex(CinematicBossGroupIndex);
		return;
	}

	StateComp->SetIdle();
	SetGravity(1.0);

	if (IsValid(BossInfoUi))
		BossInfoUi->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void ACDragon::SetGroupIndex(const int32 InIndex)
{
	if (false == IsValid(CharacterInfoComp))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ACDragon] SetGroupIndex: CharacterInfoComp missing - %s."),
			*GetNameSafe(this));
		return;
	}

	CharacterInfoComp->SetCharacterGroup(InIndex);
}

bool ACDragon::IsFlying() const
{
	if (false == IsValid(StateComp))
		return false;

	return StateComp->IsFlying() || StateComp->IsFalling();
}

void ACDragon::BP_ClearGroggyGaugeOnly()
{
	Groggy = 0.0;
}

void ACDragon::SetGroggyDamage()
{
	const float d = HitData.Damage;
	if (d <= KINDA_SMALL_NUMBER)
		return;

	const double divisor =
		FMath::Max(1.0, GroggyDamageDivisorFromHitDamage);

	const double contributed =
		static_cast<double>(d) / divisor;

	Groggy = FMath::Clamp(Groggy + contributed, 0.0, MaxGroggy);

	// Equivalent to legacy BP GreaterEqual(Groggy, MaxGroggy) after clamp ceiling.
	const bool groggyBurst =
		MaxGroggy > static_cast<double>(KINDA_SMALL_NUMBER)
		&& (Groggy + 1.e-6 >= MaxGroggy);

	if (false == groggyBurst)
		return;

	Groggy = 0.0;
	InGroggy = true;

	if (IsValid(StateComp))
		StateComp->SetGroggy();

	StopAnimMontage(nullptr);

	if (IsValid(GroggyAnim))
		PlayAnimMontage(GroggyAnim.Get(), 1.0f);

	DragonTrySetWeaponInActionBlueprint(false);

	UWorld* const world = GetWorld();
	if (nullptr == world)
		return;

	world->GetTimerManager().ClearTimer(DragonGroggyRecoverTimerHandle);

	FTimerDelegate del;
	del.BindUObject(this, &ACDragon::Dragon_OnGroggyRecoverTimer_EndGroggy);

	const float groggyRecoverDelaySeconds =
		static_cast<float>(FMath::Max(0.0, GroggyTime + 1.0));

	world->GetTimerManager().SetTimer(DragonGroggyRecoverTimerHandle, del, groggyRecoverDelaySeconds, false);
}

void ACDragon::End_Groggy()
{
	InGroggy = false;

	if (IsValid(StateComp))
		StateComp->SetIdle();
}

void ACDragon::Dragon_OnGroggyRecoverTimer_EndGroggy()
{
	End_Groggy();
}

void ACDragon::DragonTrySetWeaponInActionBlueprint(const bool InAction)
{
	if (false == IsValid(DragonWeapon))
		return;

	UClass* const weaponClass = DragonWeapon->GetClass();
	if (nullptr == weaponClass)
		return;

	FBoolProperty* const inActionProp = CastField<FBoolProperty>(
		weaponClass->FindPropertyByName(FName(TEXT("InAction"))));

	if (nullptr == inActionProp)
	{
		UE_LOG(
			LogTemp,
			Verbose,
			TEXT("[ACDragon] DragonWeapon InAction BP property missing (%s)."),
			*GetNameSafe(DragonWeapon));
		return;
	}

	inActionProp->SetPropertyValue_InContainer(DragonWeapon, InAction);
}

void ACDragon::UpdateHp()
{
	RefreshDragonBossHpBarWidgets();
}

void ACDragon::UpdateGroggy()
{
	RefreshDragonGroggyBarWidgets();
}

void ACDragon::SpawnMessage()
{
	if (false == IsValid(SystemMessageComp))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ACDragon] SpawnMessage: SystemMessageComp missing - %s."),
			*GetNameSafe(this));
		return;
	}

	SystemMessageComp->Play(DragonSpawnAnnouncementMessage, DragonSpawnAnnouncementDurationSeconds);
}

void ACDragon::InvokeHittedEffects()
{
	Super::InvokeHittedEffects();

	SetGroggyDamage();
	RefreshDragonBossHpBarWidgets();
	RefreshDragonGroggyBarWidgets();
}

void ACDragon::RefreshDragonBossHpBarWidgets()
{
	SetHp();

	UUserWidget* const bar = Cast<UUserWidget>(HpBarUi.Get());
	if (false == IsValid(bar))
		return;

	const double pct =
		(MaxHp > static_cast<double>(KINDA_SMALL_NUMBER))
			? FMath::Clamp(Hp / MaxHp, 0.0, 1.0)
			: 0.0;

	CallBossWidgetHpPercent(bar, pct);
	CallBossWidgetSetHpUi(bar, Hp, MaxHp);
}

void ACDragon::RefreshDragonGroggyBarWidgets()
{
	UUserWidget* const bar = Cast<UUserWidget>(GroggyBarUi.Get());
	if (false == IsValid(bar))
		return;

	InvokeBossGroggyBarRefresh(bar, Groggy, MaxGroggy);
}

void ACDragon::Dead()
{
	UWorld* const world = GetWorld();
	if (nullptr != world)
	{
		world->GetTimerManager().ClearTimer(DragonGroggyRecoverTimerHandle);
	}

	Super::Dead();

	SetGroupIndex(0);

	if (IsValid(BossInfoUi))
	{
		BossInfoUi->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ACDragon::End_Dead()
{
	UWorld* const world = GetWorld();
	if (nullptr == world)
	{
		Destroy();
		return;
	}

	// ??? ??: ACAnimal ? ?? Soul ??? ?? 2.5 ? ? ?? ?? ? ??????(BP_Dragon).
	FTimerDelegate del;
	del.BindUObject(this, &ACDragon::Dragon_OnEndDeadAfterSoulDelay);
	world->GetTimerManager().SetTimer(DragonEndDeadDelayTimer, del, 2.5f, false);
}

void ACDragon::Dragon_OnEndDeadAfterSoulDelay()
{
	UWorld* const world = GetWorld();
	const FVector atLocation = GetActorLocation();

	if (nullptr != world && IsValid(DeathSoulEmitterTemplate))
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			world,
			DeathSoulEmitterTemplate.Get(),
			atLocation,
			FRotator::ZeroRotator,
			FVector::OneVector,
			true,
			EPSCPoolMethod::None,
			true);
	}

	TSubclassOf<AActor> followerClass = DragonDeathSoulFollowerClass;
	if (followerClass == nullptr)
	{
		followerClass = GetClass();
	}

	if (nullptr != world && followerClass != nullptr)
	{
		FActorSpawnParameters spawnParams;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		world->SpawnActor<AActor>(followerClass, GetActorTransform(), spawnParams);
	}

	Destroy();
}

void ACDragon::InvokeDragonWeaponSkill(const uint8 InSkillEnumValue)
{
	if (false == IsValid(DragonWeapon))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ACDragon] DragonWeapon missing - skip Skill(%u) (%s)."),
			static_cast<uint32>(InSkillEnumValue),
			*GetNameSafe(this));
		return;
	}

	ACDragonWeapon* const dragonWeaponScratch = Cast<ACDragonWeapon>(DragonWeapon.Get());
	if (nullptr != dragonWeaponScratch)
	{
		dragonWeaponScratch->Skill(InSkillEnumValue);
		return;
	}

	static const FName SkillName(TEXT("Skill"));
	UFunction* const fn = DragonWeapon->FindFunction(SkillName);
	if (nullptr == fn)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ACDragon] DragonWeapon.Skill Blueprint event missing - %s."),
			*GetNameSafe(DragonWeapon));
		return;
	}

	struct FDragonSkillParams
	{
		uint8 InSkillType;
	};
	FDragonSkillParams payload{};
	payload.InSkillType = InSkillEnumValue;

	DragonWeapon->ProcessEvent(fn, &payload);
}

bool ACDragon::Server_RequestDragonWeaponSkill_Validate(uint8 InSkillEnumValue)
{
	return InSkillEnumValue < static_cast<uint8>(CEDragonBossSkillType::Max);
}

void ACDragon::Server_RequestDragonWeaponSkill_Implementation(uint8 InSkillEnumValue)
{
	InvokeDragonWeaponSkill(InSkillEnumValue);
}

void ACDragon::Dragon_InputDragonRoarPressed()
{
	const uint8 skillByteScratch = static_cast<uint8>(CEDragonBossSkillType::Roar);
	if (HasAuthority())
	{
		InvokeDragonWeaponSkill(skillByteScratch);
	}
	else
	{
		Server_RequestDragonWeaponSkill(skillByteScratch);
	}
}

void ACDragon::Dragon_InputDragonFirePressed()
{
	const uint8 skillByteScratch = static_cast<uint8>(CEDragonBossSkillType::Fire);
	if (HasAuthority())
	{
		InvokeDragonWeaponSkill(skillByteScratch);
	}
	else
	{
		Server_RequestDragonWeaponSkill(skillByteScratch);
	}
}

void ACDragon::PlayDragonFootstepLandCameraShakeAndWeapon()
{
	PlayDragonLandCameraShakePlayer0Local();
	(void)TryFireDragonWeaponLandAttackBlueprint(TEXT("Footstep extras"));
}

void ACDragon::PlayDragonLandCameraShakePlayer0Local()
{
	if (nullptr == LandCameraShakeClass)
	{
		return;
	}

	UWorld* const world = GetWorld();
	if (nullptr == world)
	{
		return;
	}

	if (world->IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	APlayerController* const pc = UGameplayStatics::GetPlayerController(world, 0);
	if (nullptr == pc)
	{
		return;
	}

	APlayerCameraManager* const cam = pc->PlayerCameraManager;
	if (nullptr == cam)
	{
		return;
	}

	cam->StartCameraShake(
		LandCameraShakeClass,
		1.0f,
		ECameraShakePlaySpace::CameraLocal,
		FRotator::ZeroRotator);
}

bool ACDragon::TryFireDragonWeaponLandAttackBlueprint(const TCHAR* CallerContextTag)
{
	if (false == IsValid(DragonWeapon))
	{
		return false;
	}

	ACDragonWeapon* const dragonWeaponScratch = Cast<ACDragonWeapon>(DragonWeapon.Get());
	if (nullptr != dragonWeaponScratch)
	{
		dragonWeaponScratch->LandAttack();
		return true;
	}

	static const FName landAttack(TEXT("LandAttack"));
	UFunction* const fn = DragonWeapon->FindFunction(landAttack);
	if (nullptr == fn)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ACDragon] DragonWeapon LandAttack BP event missing (%s) - %s / %s."),
			CallerContextTag,
			*GetNameSafe(DragonWeapon),
			*GetNameSafe(this));
		return false;
	}

	DragonWeapon->ProcessEvent(fn, nullptr);
	return true;
}

void ACDragon::ApplyNeckAttachToDragonMeshSocketFireGround()
{
	ApplyNeckAttachToDragonMeshSocket(FName(TEXT("Fire_Ground")));
}

void ACDragon::ApplyNeckAttachToDragonMeshSocket(const FName SocketName)
{
	if (false == IsValid(Neck))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ACDragon] Neck missing - skip Neck socket attach (%s)."),
			*GetNameSafe(this));
		return;
	}

	USkeletalMeshComponent* const mesh = GetMesh();
	if (nullptr == mesh)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ACDragon] Mesh missing - skip Neck attach (%s)."),
			*GetNameSafe(this));
		return;
	}

	if (true == SocketName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ACDragon] SocketName 비어 있음 — Neck 부착 생략 (%s)."), *GetNameSafe(this));
		return;
	}

	const FAttachmentTransformRules rules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepRelative,
		true);

	Neck->AttachToComponent(mesh, rules, SocketName);
}

void ACDragon::InvokeAnimalLandingAudioVisualOnlyForDragon()
{
	if (true == IsValid(LandSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, LandSound, GetActorLocation());
	}

	FTransform landEffectTransform = GetActorTransform();
	landEffectTransform.SetScale3D(landEffectTransform.GetScale3D() * LandEffectScaleFactor);

	if (true == IsValid(LandEffect))
	{
		YJJHelpers::PlayEffect(GetWorld(), LandEffect, landEffectTransform);
	}
}

void ACDragon::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	UCMovementComponent* const resolvedMovement = EnsureMovementComp();
	if (IsValid(resolvedMovement))
	{
		PlayerInputComponent->BindAction("Walk", IE_Pressed, resolvedMovement, &UCMovementComponent::InputAction_Walk);
		PlayerInputComponent->BindAction("Walk", IE_Released, resolvedMovement, &UCMovementComponent::InputAction_Run);
	}
	else if (IsLocallyControlled())
	{
		CLog::Log(FString::Printf(
			TEXT("[입력 바인딩] MovementComp 없음 — Walk 미바인딩. BP 레거시 컴포넌트 중복 여부 확인. Actor=%s"),
			*GetNameSafe(this)));
	}

	// 공중 입력은 클래스 내부 Fly/Move 핸들러로 묶임.
	PlayerInputComponent->BindAxis("MoveForward", this, &ACDragon::Dragon_InputAxis_MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACDragon::Dragon_InputAxis_MoveRight);
	PlayerInputComponent->BindAxis("HorizontalLook", FlyComp.Get(), &UCFlyComponent::InputAxis_HorizontalLook);
	PlayerInputComponent->BindAxis("VerticalLook", FlyComp.Get(), &UCFlyComponent::InputAxis_VerticalLook);
	PlayerInputComponent->BindAxis("FlyUp", this, &ACDragon::Dragon_InputAxis_FlyUp);
	PlayerInputComponent->BindAxis("Zoom", CamComp.Get(), &UCCamComponent::InputAxis_Zoom);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACDragon::Dragon_InputAction_Jump);
	PlayerInputComponent->BindAction("Targeting", IE_Pressed, TargetingComp.Get(), &UCTargetingComponent::InputAction_Targeting);
	PlayerInputComponent->BindAction("Menu", IE_Pressed, GameUIComp.Get(), &UCGameUIComponent::InputAction_ActivateEquipMenu);
	PlayerInputComponent->BindAction("Menu", IE_Released, GameUIComp.Get(), &UCGameUIComponent::InputAction_DeactivateEquipMenu);
	PlayerInputComponent->BindAction("Action", IE_Pressed, AnimalRidingComponent.Get(), &UCRidingComponent::InputAction_Act);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ACCommonCharacter::InputAction_Interact);
	PlayerInputComponent->BindAction("DragonRoar", IE_Pressed, this, &ACDragon::Dragon_InputDragonRoarPressed);
	PlayerInputComponent->BindAction("DragonFire", IE_Pressed, this, &ACDragon::Dragon_InputDragonFirePressed);
}

bool ACDragon::Dragon_ShouldApplyMovingGraphAirAssist() const
{
	// ??? BP IsFlying ?? ? ?? ?? ??? FallMode ? ?? ??.
	return IsValid(StateComp) && StateComp->IsFallMode();
}

void ACDragon::Dragon_InputAxis_MoveForward(const float AxisValue)
{
	bool bAxisCountsForCanMove = false;
	CanMove(static_cast<double>(AxisValue), bAxisCountsForCanMove);
	if (false == bAxisCountsForCanMove)
		return;

	FVector outForward = FVector::ZeroVector;
	FVector outRight = FVector::ZeroVector;
	GetControlDirection(outForward, outRight);

	AddMovementInput(outForward, AxisValue, true);

	if (false == Dragon_ShouldApplyMovingGraphAirAssist())
		return;

	const UCapsuleComponent* capsule = GetCapsuleComponent();
	if (nullptr == capsule)
		return;

	const FVector horizontalNudge =
		capsule->GetForwardVector() * AxisValue * DragonMovingGraphStrafeUnitsPerPoll;

	SetActorLocation(GetActorLocation() + horizontalNudge);
}

void ACDragon::Dragon_InputAxis_MoveRight(const float AxisValue)
{
	bool bAxisCountsForCanMove = false;
	CanMove(static_cast<double>(AxisValue), bAxisCountsForCanMove);

	if (false == IsValid(StateComp))
		return;

	UWorld* const world = GetWorld();
	if (nullptr == world)
		return;

	const float deltaSeconds = UGameplayStatics::GetWorldDeltaSeconds(world);

	FVector outForward = FVector::ZeroVector;
	FVector outRight = FVector::ZeroVector;
	GetControlDirection(outForward, outRight);

	if (true == bAxisCountsForCanMove)
	{
		AddMovementInput(outRight, AxisValue, true);

		if (true == Dragon_ShouldApplyMovingGraphAirAssist())
		{
			const UCapsuleComponent* capsule = GetCapsuleComponent();
			if (nullptr != capsule)
			{
				const float absAxis = FMath::Abs(AxisValue);
				const FVector horizontalNudge =
					capsule->GetForwardVector() * absAxis * DragonMovingGraphStrafeUnitsPerPoll;

				const FRotator currentRotation = GetActorRotation();
				const float rollClamped = FMath::Clamp(
					currentRotation.Roll + AxisValue,
					-DragonMovingGraphAirRollClampDegrees,
					DragonMovingGraphAirRollClampDegrees);

				// ??? MakeRotator ? Roll ???, Pitch/Yaw ??
				const FRotator blendedRotation(
					currentRotation.Pitch,
					currentRotation.Yaw,
					rollClamped);

				FTransform nextTransform(GetActorTransform());
				nextTransform.SetLocation(GetActorLocation() + horizontalNudge);
				nextTransform.SetRotation(FQuat(blendedRotation));

				SetActorTransform(nextTransform);
			}
		}
	}
	else if (true == Dragon_ShouldApplyMovingGraphAirAssist())
	{
		// ??? ? CanMove=false ?? ???? ??/??? ??? ??
		const FRotator currentRotation = GetActorRotation();
		const FRotator targetRotation(0.0f, currentRotation.Yaw, currentRotation.Roll);

		SetActorRotation(UKismetMathLibrary::RInterpTo(
			currentRotation,
			targetRotation,
			deltaSeconds,
			DragonMovingGraphLevelingInterpSpeed));
	}
}

void ACDragon::Dragon_InputAxis_FlyUp(const float AxisValue)
{
	bool bAxisCountsForCanMove = false;
	CanMove(static_cast<double>(AxisValue), bAxisCountsForCanMove);

	UWorld* const world = GetWorld();
	if (nullptr == world)
		return;

	const float deltaSeconds = UGameplayStatics::GetWorldDeltaSeconds(world);

	if (true == bAxisCountsForCanMove)
	{
		const float previousUpFactor = UpFactor;

		if (AxisValue > 0.0f)
		{
			UpFactor = 20.0f;
		}
		else
		{
			UpFactor = -50.0f;
		}

		if (false == FMath::IsNearlyEqual(previousUpFactor, UpFactor))
		{
			SyncFlyVerticalStrengthFromUpFactor();
		}

		const UCapsuleComponent* capsule = GetCapsuleComponent();
		if (nullptr != capsule)
		{
			const FVector newLocation =
				GetActorLocation() + capsule->GetForwardVector() * AxisValue * UpFactor;

			const FRotator currentRotation = GetActorRotation();

			const float pitchClamped = FMath::Clamp(
				currentRotation.Pitch + AxisValue,
				-DragonMovingGraphAirPitchClampDegrees,
				DragonMovingGraphAirPitchClampDegrees);

			// ??? MakeRotator ? Pitch ???, Yaw/Roll ??
			const FRotator blendedRotation(pitchClamped, currentRotation.Yaw, currentRotation.Roll);

			FTransform newTransform;
			newTransform.SetLocation(newLocation);
			newTransform.SetRotation(FQuat(blendedRotation));
			newTransform.SetScale3D(GetActorScale3D());

			SetActorTransform(newTransform);
		}
	}
	else if (true == Dragon_ShouldApplyMovingGraphAirAssist())
	{
		const FRotator currentRotation = GetActorRotation();
		const FRotator targetRotation(0.0f, currentRotation.Yaw, currentRotation.Roll);

		SetActorRotation(UKismetMathLibrary::RInterpTo(
			currentRotation,
			targetRotation,
			deltaSeconds,
			DragonMovingGraphLevelingInterpSpeed));
	}
}

void ACDragon::Dragon_InputAction_Jump()
{
	UWorld* const world = GetWorld();
	if (nullptr == world)
		return;

	// ??? FlyJump ? ?? ?? ??? ?? LandOn ??
	if (true == Dragon_ShouldApplyMovingGraphAirAssist())
	{
		if (true == IsValid(FlyComp))
			FlyComp->LandOn();
		return;
	}

	if (false == IsValid(MovementComp) || false == MovementComp->CanMove())
		return;

	if (false == IsValid(StateComp))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[ACDragon] Dragon_InputAction_Jump: StateComp ?? - ?? ?? ?? (%s)."),
			*GetNameSafe(this));
		return;
	}

	ACharacter::Jump();

	// FlyJump BP ???: Flying ?? + ?? ??
	StateComp->SetFlying();
	MovementComp->SetGravity(DragonJumpBlueprintGravity);

	// Niagara ?? FX ? ?? ?? ?????(??? BP ?? ??)
	const bool spawnJumpFx = true == IsLocallyControlled();

	if ((true == spawnJumpFx) && true == IsValid(JumpEffect) && nullptr != GetMesh())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			world,
			JumpEffect,
			GetMesh()->GetComponentLocation(),
			FRotator::ZeroRotator,
			FVector::OneVector,
			true,
			true);
	}
}

void ACDragon::Landed(const FHitResult& Hit)
{
	// ??? BP Footstep/Land(Falling ?? ??? ???) ?? ? LandAt?Montage ?. ?? Landed ? Idle ???? ????.
	if (nullptr == StateComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ACDragon] Landed: StateComp ?? - ACAnimal ?? ?? (%s)."), *GetNameSafe(this));

		Super::Landed(Hit);

		if (IsValid(FlyComp))
			FlyComp->LandOn();
		return;
	}

	const bool wasFallingMode = StateComp->IsFallMode();
	const bool wasCinematicMode = StateComp->IsCinematic();

	if (false == wasFallingMode && false == wasCinematicMode)
	{
		Super::Landed(Hit);

		if (IsValid(FlyComp))
			FlyComp->LandOn();
		return;
	}

	if (true == wasFallingMode)
	{
		StateComp->SetIdleMode();
	}

	if (true == wasCinematicMode)
	{
		StateComp->SetCinematic();
	}

	SetGravity(1.0);

	const FVector stepWorld = GetActorLocation();
	LandAt(EPhysicalSurface::SurfaceType_Default, stepWorld);

	PlayDragonLandCameraShakePlayer0Local();
	(void)TryFireDragonWeaponLandAttackBlueprint(TEXT("Landing (Fall/Cinematic)"));

	ACharacter::Landed(Hit);

	if (IsValid(MovementComp))
	{
		MovementComp->SetGravity(1.f);
	}

	PlayLandMontageIfAny();

	InvokeAnimalLandingAudioVisualOnlyForDragon();

	if (IsValid(FlyComp))
	{
		FlyComp->LandOn();
	}
}
