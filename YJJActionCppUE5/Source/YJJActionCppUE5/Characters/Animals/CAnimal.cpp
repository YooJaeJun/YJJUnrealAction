#include "Characters/Animals/CAnimal.h"
#include "Components/CCharacterStatComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CMontagesComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Particles/ParticleSystem.h"
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
}

UWidgetComponent* ACAnimal::GetAnimalHpBarWidgetComponent() const
{
	return nullptr;
}

void ACAnimal::BeginPlay()
{
	Super::BeginPlay();

	// StatComp 기반 Hp 미러 후 UI 갱신. HpBarWidgetComp 는 자식 BeginPlay 에서 SetWidgetClass 될 수 있어 AI 는 끝에서 SetHpUI 를 한 번 더 호출한다.
	SetHp();
	SetHpUI();
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
		dmg = HitData.Power;

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

void ACAnimal::Hitted()
{
	bool bAlive = false;
	SetDamage(0.0f, bAlive);

	if (bAlive)
	{
		if (IsValid(StateComp))
			StateComp->SetHitted();

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

	IsDead.Broadcast();

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

void ACAnimal::Begin_Dead()
{
	Dead();
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

void ACAnimal::Footstep_Implementation()
{
	// 레거시 I_Character Footstep 는 파라미터가 있어 AnimNotify 에서 FootstepAt 을 직접 호출한다.
}

void ACAnimal::ToggleIK_Implementation()
{
	// ICInterface_IK 의 알파를 토글 — 탑승 시 RidingComp 가 별도 값을 줄 수 있다.
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

FVector ACAnimal::GetDesiredMovement_Implementation()
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

	return combined * speedMul;
}

void ACAnimal::SetFootLocation_Implementation(bool bLeftFoot, FVector WorldLocation)
{
	// 구 BP 는 AnimBP·풋 IK 타깃 전달용 — 네이티브 기본은 노옵(블루프린트에서 확장).
	(void)bLeftFoot;
	(void)WorldLocation;
}

void ACAnimal::PlayHitStop()
{
	if (FMath::IsNearlyZero(HitData.StopTime))
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
	world->GetTimerManager().SetTimer(HitStopRestoreTimer, restoreDel, HitData.StopTime, false);
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
