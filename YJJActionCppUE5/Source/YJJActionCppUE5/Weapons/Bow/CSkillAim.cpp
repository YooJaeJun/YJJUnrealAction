#include "Weapons/Bow/CSkillAim.h"

#include "Characters/CCommonCharacter.h"
#include "Curves/CurveFloat.h"

#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Blueprint/UserWidget.h"
#include "Utilities/CLog.h"
#include "Utilities/YJJHelpers.h"

ACSkillAim::ACSkillAim()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	AimTraceObjectTypes.Reset();

	TEnumAsByte<EObjectTypeQuery> FourthScratch = EObjectTypeQuery::ObjectTypeQuery4;
	TEnumAsByte<EObjectTypeQuery> ThirdScratch = EObjectTypeQuery::ObjectTypeQuery3;
	AimTraceObjectTypes.Add(FourthScratch);
	AimTraceObjectTypes.Add(ThirdScratch);

	YJJHelpers::GetClass<UUserWidget>(
		&CrossHairWidgetClass,
		TEXT("WidgetBlueprint'/Game/Widgets/Weapons/WB_CrossHair.WB_CrossHair_C'"));
}

void ACSkillAim::BeginPlay()
{
	Super::BeginPlay();

	SkillAim_ResolveSpringArmAndCameraScratch();
	SkillAim_ResolveControllerScratch();
	SkillAim_RefreshNeedsTickScratch();
}

void ACSkillAim::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SkillAim_TickFoVInterpScratch(DeltaSeconds);

	if (false == Aiming)
	{
		return;
	}

	SkillAim_TickCrosshairRayScratch();
}

void ACSkillAim::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(CrossHair))
	{
		CrossHair->RemoveFromParent();
		CrossHair = nullptr;
	}

	FoVInterpScratch = EAimFoVInterpKind::None;
	SetActorTickEnabled(false);

	Super::EndPlay(EndPlayReason);
}

void ACSkillAim::Pressed_Implementation()
{
	SkillAim_OnPressedFlowScratch();
}

void ACSkillAim::Released_Implementation()
{
	SkillAim_OnReleasedFlowScratch();
}

void ACSkillAim::SkillAim_RefreshNeedsTickScratch()
{
	const bool bNeedTickScratch = (false != Aiming) || (FoVInterpScratch != EAimFoVInterpKind::None);
	SetActorTickEnabled(bNeedTickScratch);
}

bool ACSkillAim::SkillAim_ResolveSpringArmAndCameraScratch()
{
	SpringArm = nullptr;
	Camera = nullptr;

	if (false == IsValid(Character))
	{
		return false;
	}

	SpringArm = Character->FindComponentByClass<USpringArmComponent>();
	Camera = Character->FindComponentByClass<UCameraComponent>();

	if (false == IsValid(SpringArm))
	{
		CLog::Log(FString::Printf(TEXT("[SkillAim] SpringArmComponent 없음 — %s"), *GetNameSafe(this)));
	}

	if (false == IsValid(Camera))
	{
		CLog::Log(FString::Printf(TEXT("[SkillAim] CameraComponent 없음 — %s"), *GetNameSafe(this)));
	}

	return IsValid(SpringArm) && IsValid(Camera);
}

void ACSkillAim::SkillAim_ResolveControllerScratch()
{
	Controller = nullptr;

	if (false == IsValid(Character))
	{
		return;
	}

	TWeakObjectPtr<AController> WeakScratch = Character->GetMyCurController();
	if (false == WeakScratch.IsValid())
	{
		WeakScratch = Character->GetController();
	}

	if (WeakScratch.IsValid())
	{
		Controller = WeakScratch.Get();
	}
}

bool ACSkillAim::SkillAim_EnsureCrosshairWidgetScratch()
{
	if (IsValid(CrossHair))
	{
		return true;
	}

	if (false == IsValid(Character))
	{
		return false;
	}

	SkillAim_ResolveControllerScratch();

	APlayerController* PlayerControllerScratch = Cast<APlayerController>(Controller.Get());
	if (false == IsValid(PlayerControllerScratch))
	{
		// 블프 주석: Riding 등에서 CurController 가 플레이어 PC 가 아닐 수 있음 — 스팸 방지 로그 없음.
		return false;
	}

	if (false == IsValid(CrossHairWidgetClass))
	{
		CLog::Log(FString::Printf(TEXT("[SkillAim] CrossHairWidgetClass 미설정 — %s"), *GetNameSafe(this)));
		return false;
	}

	CrossHair = CreateWidget<UUserWidget>(PlayerControllerScratch, CrossHairWidgetClass);
	if (false == IsValid(CrossHair))
	{
		CLog::Log(FString::Printf(TEXT("[SkillAim] CrossHair 위젯 생성 실패 — %s"), *GetNameSafe(this)));
		return false;
	}

	CrossHair->AddToViewport(0);
	return true;
}
void ACSkillAim::SkillAim_CallCrossHairUiEventIfDefined(UUserWidget* WidgetScratch, const FName& FunctionNameScratch)
{
	if (false == IsValid(WidgetScratch))
	{
		return;
	}

	UFunction* FunctionResolvedScratch = WidgetScratch->FindFunction(FunctionNameScratch);
	if (nullptr == FunctionResolvedScratch)
	{
		return;
	}

	WidgetScratch->ProcessEvent(FunctionResolvedScratch, nullptr);
}

void ACSkillAim::SkillAim_TryLoadBowPullSoundScratch()
{
	if (IsValid(AimBowPullSound))
	{
		return;
	}

	TObjectPtr<USoundBase> LoadedScratch = nullptr;
	YJJHelpers::GetAssetDynamic<USoundBase>(
		&LoadedScratch,
		FString(TEXT("/Game/Assets/Sounds/Weapons/Bow/Bow_Pull.Bow_Pull")));
	AimBowPullSound = LoadedScratch.Get();
}

void ACSkillAim::SkillAim_StartFoVInterpScratch(bool bZoomInScratch)
{
	if (false == IsValid(Camera))
	{
		FoVInterpScratch = EAimFoVInterpKind::None;
		SkillAim_RefreshNeedsTickScratch();
		return;
	}

	FoVInterpElapsedScratch = 0.f;

	if (false != bZoomInScratch)
	{
		CachedFoVBeforeAimScratch = Camera->FieldOfView;
		FoVInterpScratch = EAimFoVInterpKind::ZoomIn;
	}
	else
	{
		CachedFoVAtZoomOutStartScratch = Camera->FieldOfView;
		FoVInterpScratch = EAimFoVInterpKind::ZoomOut;
	}

	SkillAim_RefreshNeedsTickScratch();
}

float ACSkillAim::SkillAim_AlphaNormalizedFromInterpScratch(
	float ElapsedScratch,
	float DurationScratch,
	const UCurveFloat* CurveScratch) const
{
	if (DurationScratch <= KINDA_SMALL_NUMBER)
	{
		return 1.f;
	}

	const float LinearAlphaScratch = FMath::Clamp(ElapsedScratch / DurationScratch, 0.f, 1.f);

	if (nullptr == CurveScratch)
	{
		return LinearAlphaScratch;
	}

	return FMath::Clamp(CurveScratch->GetFloatValue(LinearAlphaScratch), 0.f, 1.f);
}

void ACSkillAim::SkillAim_TickFoVInterpScratch(float DeltaSeconds)
{
	if (FoVInterpScratch == EAimFoVInterpKind::None)
	{
		return;
	}

	if (false == IsValid(Camera))
	{
		FoVInterpScratch = EAimFoVInterpKind::None;
		SkillAim_RefreshNeedsTickScratch();
		return;
	}

	const float DurInScratch = FMath::Max(KINDA_SMALL_NUMBER, AimZoomFoVInterpSecondsIn);
	const float DurOutScratch = FMath::Max(KINDA_SMALL_NUMBER, AimZoomFoVInterpSecondsOut);

	float UsedDurScratch = DurInScratch;
	const UCurveFloat* UsedCurveScratch = AimZoomFoVCurveIn.Get();
	if (FoVInterpScratch == EAimFoVInterpKind::ZoomOut)
	{
		UsedDurScratch = DurOutScratch;
		UsedCurveScratch = AimZoomFoVCurveOut.Get();
	}

	FoVInterpElapsedScratch += DeltaSeconds;

	const float AlphaNormScratch = SkillAim_AlphaNormalizedFromInterpScratch(
		FoVInterpElapsedScratch,
		UsedDurScratch,
		UsedCurveScratch);

	if (AlphaNormScratch >= 1.f - KINDA_SMALL_NUMBER)
	{
		if (FoVInterpScratch == EAimFoVInterpKind::ZoomIn)
		{
			Camera->SetFieldOfView(AimZoomTargetFoV);
		}
		else
		{
			Camera->SetFieldOfView(CachedFoVBeforeAimScratch);
		}

		FoVInterpScratch = EAimFoVInterpKind::None;
		FoVInterpElapsedScratch = 0.f;
		SkillAim_RefreshNeedsTickScratch();
		return;
	}

	if (FoVInterpScratch == EAimFoVInterpKind::ZoomIn)
	{
		const float FoVLerpScratch = FMath::Lerp(CachedFoVBeforeAimScratch, AimZoomTargetFoV, AlphaNormScratch);
		Camera->SetFieldOfView(FoVLerpScratch);

		if (AlphaNormScratch > 0.f)
		{
			if (false != SkillAim_PendingRevealCrossHairAfterZoomInScratch)
			{
				if (SkillAim_EnsureCrosshairWidgetScratch() != false)
				{
					CrossHair->SetVisibility(ESlateVisibility::Visible);
				}

				SkillAim_TryLoadBowPullSoundScratch();
				if ((false == SkillAim_BowPullSoundPlayedForThisPressScratch) && IsValid(AimBowPullSound) &&
					IsValid(Character))
				{
					SkillAim_BowPullSoundPlayedForThisPressScratch = true;
					UGameplayStatics::PlaySoundAtLocation(
						this,
						AimBowPullSound.Get(),
						Character->GetActorLocation());
				}
			}
		}
	}
	else
	{
		const float FoVOutScratch = FMath::Lerp(CachedFoVAtZoomOutStartScratch, CachedFoVBeforeAimScratch, AlphaNormScratch);
		Camera->SetFieldOfView(FoVOutScratch);
	}
}

void ACSkillAim::SkillAim_TickCrosshairRayScratch()
{
	if (false == IsValid(CrossHair))
	{
		return;
	}

	// UE5 슬레이트: 과거 HitTestSelf 는 제거되었다. Visible 과 SelfHitTestInvisible 은 레거시 "보이지만 클립/히트 테스트가 다른" 패턴 근사.
	if (CrossHair->GetVisibility() != ESlateVisibility::Visible &&
		CrossHair->GetVisibility() != ESlateVisibility::SelfHitTestInvisible)
	{
		return;
	}

	if (false == IsValid(Character))
	{
		return;
	}

	ACharacter* CharacterActorScratch = Cast<ACharacter>(Character);
	if (false == IsValid(CharacterActorScratch))
	{
		return;
	}

	USkeletalMeshComponent* MeshScratch = CharacterActorScratch->GetMesh();
	if (false == IsValid(MeshScratch))
	{
		return;
	}

	if (false == AimTraceObjectTypes.Num())
	{
		return;
	}

	const FVector StartScratch = MeshScratch->GetSocketLocation(AimTraceBowSocketName);

	FVector DirScratch = FVector::ForwardVector;
	if (IsValid(Controller.Get()))
	{
		AController* const controllerScratch = Controller.Get();
		APawn* const pawnScratch = controllerScratch != nullptr ? controllerScratch->GetPawn() : nullptr;
		if (IsValid(pawnScratch))
		{
			DirScratch = pawnScratch->GetActorForwardVector();
		}
		else
		{
			DirScratch = controllerScratch->GetControlRotation().Vector();
		}
	}

	const FVector EndScratch = StartScratch + (DirScratch * AimTraceLength);

	TArray<AActor*> IgnoresScratch;
	IgnoresScratch.Add(CharacterActorScratch);

	FHitResult HitResultScratch;
	const bool bLineHitScratch = UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		StartScratch,
		EndScratch,
		AimTraceObjectTypes,
		false,
		IgnoresScratch,
		EDrawDebugTrace::None,
		HitResultScratch,
		true /* bIgnoreSelf */);

	if ((false != bLineHitScratch) && (false != HitResultScratch.bBlockingHit))
	{
		ACharacter* OtherCharScratch = Cast<ACharacter>(HitResultScratch.GetActor());
		if (IsValid(OtherCharScratch))
		{
			SkillAim_CallCrossHairUiEventIfDefined(CrossHair, FName(TEXT("CrossHair_Red")));
		}
		else
		{
			SkillAim_CallCrossHairUiEventIfDefined(CrossHair, FName(TEXT("CrossHair_White")));
		}
	}
	else
	{
		SkillAim_CallCrossHairUiEventIfDefined(CrossHair, FName(TEXT("CrossHair_White")));
	}
}

void ACSkillAim::SkillAim_OnPressedFlowScratch()
{
	SkillAim_ResolveControllerScratch();

	// 줌 해제 블랜드 중 재 Press — 블프에 없던 경계지만 FoV 상태를 즉시 정리하지 않으면 새 조준 깜빡임 발생.
	if (FoVInterpScratch == EAimFoVInterpKind::ZoomOut)
	{
		if (IsValid(Camera))
		{
			Camera->SetFieldOfView(CachedFoVBeforeAimScratch);
		}
		FoVInterpScratch = EAimFoVInterpKind::None;
		FoVInterpElapsedScratch = 0.f;
		SkillAim_RefreshNeedsTickScratch();
	}

	if (false == SkillAim_ResolveSpringArmAndCameraScratch())
	{
		return;
	}

	if (false != Aiming)
	{
		return;
	}

	Aiming = true;
	SkillAim_SnapshotOriginFromCurrentScratch();
	SkillAim_ApplyAimDataToComponentsScratch(AimData);

	SkillAim_BowPullSoundPlayedForThisPressScratch = false;
	SkillAim_PendingRevealCrossHairAfterZoomInScratch = true;
	SkillAim_StartFoVInterpScratch(true /* bZoomInScratch */);
}

void ACSkillAim::SkillAim_OnReleasedFlowScratch()
{
	if (false == Aiming)
	{
		return;
	}

	Aiming = false;

	SkillAim_PendingRevealCrossHairAfterZoomInScratch = false;

	if (IsValid(CrossHair))
	{
		CrossHair->SetVisibility(ESlateVisibility::Hidden);
	}

	if (false != SkillAim_ResolveSpringArmAndCameraScratch())
	{
		SkillAim_RestoreSpringCameraFromOriginScratch();
	}

	SkillAim_StartFoVInterpScratch(false /* bZoomInScratch */);
}

void ACSkillAim::SkillAim_SnapshotOriginFromCurrentScratch()
{
	if ((false == IsValid(SpringArm)) || (false == IsValid(Camera)))
	{
		return;
	}

	Origin.TargetArmLength = SpringArm->TargetArmLength;
	Origin.SocketOffset = SpringArm->SocketOffset;
	Origin.TargetOffset = SpringArm->TargetOffset;
	Origin.bEnableCameraLag = SpringArm->bEnableCameraLag;
	Origin.CameraLocation = Camera->GetRelativeLocation();
}

void ACSkillAim::SkillAim_ApplyAimDataToComponentsScratch(const FAimData& DataScratch)
{
	if ((false == IsValid(SpringArm)) || (false == IsValid(Camera)))
	{
		return;
	}

	SpringArm->TargetArmLength = DataScratch.TargetArmLength;
	SpringArm->SocketOffset = DataScratch.SocketOffset;
	SpringArm->TargetOffset = DataScratch.TargetOffset;
	SpringArm->bEnableCameraLag = DataScratch.bEnableCameraLag;
	Camera->SetRelativeLocation(DataScratch.CameraLocation);
}

void ACSkillAim::SkillAim_RestoreSpringCameraFromOriginScratch()
{
	SkillAim_ApplyAimDataToComponentsScratch(Origin);
}
