#include "Weapons/CMagicWarpSkillContext.h"

#include "Utilities/CLog.h"
#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/DecalComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/HitResult.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

const FName ACMagicWarpSkillContext::WarpBlackboardEqLocationKey(TEXT("EQ_Location"));

ACMagicWarpSkillContext::ACMagicWarpSkillContext()
{
	WarpRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(WarpRoot);
	WarpRoot->bVisualizeComponent = true;

	WarpParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle"));
	WarpParticle->SetupAttachment(WarpRoot);
	WarpParticle->SetVisibility(false);
	WarpParticle->SetHiddenInGame(true);
	WarpParticle->SetAutoActivate(false);
	WarpParticle->PrimaryComponentTick.bStartWithTickEnabled = false;
	WarpParticle->SetComponentTickEnabled(false);

	WarpDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	WarpDecal->SetupAttachment(WarpRoot);
	WarpDecal->SetVisibility(false);
	WarpDecal->SetHiddenInGame(true);
	WarpDecal->DecalSize = FVector(10.f, 64.f, 64.f);
	WarpDecal->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
}

void ACMagicWarpSkillContext::BeginPlay()
{
	Super::BeginPlay();
	Warp_ApplyVisualTemplates();
}

void ACMagicWarpSkillContext::Equip_Implementation()
{
	Super::Equip_Implementation();

	if (false == IsValid(Character))
	{
		return;
	}

	APlayerController* pcFromCharacter = Cast<APlayerController>(Character->CurController.Get());
	if (IsValid(pcFromCharacter))
	{
		WarpPlayerController = pcFromCharacter;
	}

	Warp_SetPreviewFxVisible(true);
	Magic_FixCameraForOwnerMovement();
}

void ACMagicWarpSkillContext::Unequip_Implementation()
{
	Super::Unequip_Implementation();
	Warp_UnFixCameraMatchLegacyUnequipBranch();
	Warp_SetPreviewFxVisible(false);
}

void ACMagicWarpSkillContext::DoAction_Implementation(CEAttackType InAttackType, int32 InSkillIndex)
{
	(void)InSkillIndex;

	Super::DoAction_Implementation(InAttackType, InSkillIndex);

	const int32 RowCount = DoActionDatas.Num();
	const bool bHasRows = RowCount > 0;
	const bool bIdleOrRiding =
		true == Magic_IsOwnerIdle() || true == Magic_IsOwnerRiding();
	const bool bShouldStartWarp = true == bHasRows && true == bIdleOrRiding;

	if (true == bShouldStartWarp)
	{
		Magic_SetOwnerStateAction();

		bool bUnderCursor = false;
		FVector hitLocUnderCursor = FVector::ZeroVector;
		FRotator rotUnused = FRotator::ZeroRotator;
		GetCursorLocationAndRotation(bUnderCursor, hitLocUnderCursor, rotUnused);

		if (true == bUnderCursor)
		{
			SetCandidate(hitLocUnderCursor);
		}
		else
		{
			FVector aiLoc = FVector::ZeroVector;
			bool bAiOk = false;
			Warp_TryResolveAiWarpLocation(aiLoc, bAiOk);
			if (true == bAiOk)
			{
				SetCandidate(aiLoc);
			}
			else
			{
				CLog::Log(FString::Printf(
					TEXT("[MagicWarp] DoAction: 커서 히트·AI 블랙보드 모두 없어 SetCandidate 생략 — %s"),
					*GetNameSafe(this)));
			}
		}
		return;
	}

	Warp_EndActionAndUnequipIfBusy();
}

void ACMagicWarpSkillContext::Begin_DoAction_Implementation(CEAttackType InAttackType)
{
	Super::Begin_DoAction_Implementation(InAttackType);

	UWorld* World = GetWorld();
	if (false == IsValid(World))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicWarp] Begin_DoAction: World 무효 — 워프 지형 검사 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicWarp] Begin_DoAction: Character 무효 — 워프 지형 검사 생략 — %s"),
			*GetNameSafe(this)));
		return;
	}

	FVector elevatedPointWorld = FVector::ZeroVector;
	Warp_MakeElevatedWarpPoint(elevatedPointWorld);

	FVector probeDownEnd = elevatedPointWorld;
	probeDownEnd.Z = static_cast<float>(
		static_cast<double>(elevatedPointWorld.Z) - WarpTerrainProbeDownUU);

	FHitResult downHit;
	const bool bDownBlocking = Warp_LineTraceWarpAxis(World, elevatedPointWorld, probeDownEnd, downHit);

	if (true == bDownBlocking)
	{
		if (true == Magic_IsCharacterRealRiding())
		{
			if (false == IsValid(Character->CurInteractingActor))
			{
				CLog::Log(FString::Printf(
					TEXT("[MagicWarp] Begin_DoAction: RealRiding 이지만 CurInteractingActor 없음 — 캐릭터 분기 생략 — %s"),
					*GetNameSafe(this)));
			}
			else
			{
				Character->CurInteractingActor->SetActorLocation(elevatedPointWorld, false, nullptr, ETeleportType::None);
			}
		}
		else
		{
			Character->SetActorLocation(elevatedPointWorld, false, nullptr, ETeleportType::None);
		}
		return;
	}

	FVector probeUpEnd = elevatedPointWorld;
	probeUpEnd.Z = static_cast<float>(
		static_cast<double>(elevatedPointWorld.Z) + WarpTerrainProbeUpUU);

	FHitResult upHit;
	const bool bUpBlocking =
		Warp_LineTraceWarpAxis(World, elevatedPointWorld, probeUpEnd, upHit);

	if (false == bUpBlocking)
	{
		// 레거시: 상향에도 차단 없으면 종료만(설계상 허공 — 로그 과다 우려 없이 간단 처리).
		return;
	}

	const FVector impulse = upHit.ImpactPoint;
	ACharacter* asCharWarp = Cast<ACharacter>(Character);
	if (false == IsValid(asCharWarp))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicWarp] Begin_DoAction: Character 가 ACharacter 가 아니라 상향 보정 위치 적용 불가 — %s"),
			*GetNameSafe(this)));
		return;
	}

	UCapsuleComponent* cap = asCharWarp->GetCapsuleComponent();
	if (false == IsValid(cap))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicWarp] Begin_DoAction: CapsuleComponent 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	const double halfUnscaled = static_cast<double>(cap->GetUnscaledCapsuleHalfHeight());
	const double riseZ = halfUnscaled * 2.0;
	FVector teleportTo = impulse;
	teleportTo.Z = teleportTo.Z + static_cast<float>(riseZ);

	Character->SetActorLocation(teleportTo, false, nullptr, ETeleportType::None);

	(void)InAttackType;
}

void ACMagicWarpSkillContext::Warp_ApplyVisualTemplates()
{
	if (IsValid(WarpParticleTemplate) && IsValid(WarpParticle))
	{
		WarpParticle->SetTemplate(WarpParticleTemplate);
	}

	if (IsValid(WarpDecalMaterial) && IsValid(WarpDecal))
	{
		WarpDecal->SetDecalMaterial(WarpDecalMaterial);
	}
}

APlayerController* ACMagicWarpSkillContext::Warp_ResolvePlayerController() const
{
	if (IsValid(WarpPlayerController))
	{
		return WarpPlayerController;
	}

	if (IsValid(Controller))
	{
		return Cast<APlayerController>(Controller);
	}

	return nullptr;
}

void ACMagicWarpSkillContext::Warp_SetPreviewFxVisible(const bool bVisible)
{
	if (IsValid(WarpDecal))
	{
		if (true == bVisible)
		{
			WarpDecal->SetHiddenInGame(false);
		}
		else
		{
			WarpDecal->SetHiddenInGame(true);
		}
		WarpDecal->SetVisibility(bVisible, false);
	}

	if (IsValid(WarpParticle))
	{
		if (true == bVisible)
		{
			WarpParticle->SetHiddenInGame(false);
		}
		else
		{
			WarpParticle->SetHiddenInGame(true);
		}

		WarpParticle->SetVisibility(bVisible, false);

		if (true == bVisible)
		{
			WarpParticle->Activate(true);
		}
		else
		{
			WarpParticle->Deactivate();
		}
	}
}

void ACMagicWarpSkillContext::Warp_UnFixCameraMatchLegacyUnequipBranch()
{
	if (false == IsValid(Character))
	{
		return;
	}

	if (IsValid(Character->CurInteractingActor))
	{
		UCMovementComponent* interactMove =
			Character->CurInteractingActor->FindComponentByClass<UCMovementComponent>();
		if (IsValid(interactMove))
		{
			interactMove->UnFixCamera();
		}

		return;
	}

	if (UCMovementComponent* charMove = Character->FindComponentByClass<UCMovementComponent>())
	{
		charMove->UnFixCamera();
	}
}

void ACMagicWarpSkillContext::Warp_MakeElevatedWarpPoint(FVector& OutPoint) const
{
	OutPoint = WarpCandidateLocation;

	if (false == IsValid(Character))
	{
		OutPoint.Z = static_cast<float>(
			static_cast<double>(OutPoint.Z) + WarpElevateAboveCandidateZUU);
		return;
	}

	ACharacter* asChar = Cast<ACharacter>(Character);
	if (false == IsValid(asChar))
	{
		OutPoint.Z = static_cast<float>(
			static_cast<double>(OutPoint.Z) + WarpElevateAboveCandidateZUU);
		return;
	}

	UCapsuleComponent* capsuleResolved = asChar->GetCapsuleComponent();
	if (false == IsValid(capsuleResolved))
	{
		OutPoint.Z = static_cast<float>(
			static_cast<double>(OutPoint.Z) + WarpElevateAboveCandidateZUU);
		return;
	}

	const double capsuleHalfScaled = static_cast<double>(capsuleResolved->GetScaledCapsuleHalfHeight());
	const double newZ =
		static_cast<double>(OutPoint.Z) + capsuleHalfScaled + WarpElevateAboveCandidateZUU;
	OutPoint.Z = static_cast<float>(newZ);
}

bool ACMagicWarpSkillContext::Warp_LineTraceWarpAxis(
	UWorld* World,
	const FVector& StartWorld,
	const FVector& EndWorld,
	FHitResult& OutHit) const
{
	if (false == IsValid(World))
	{
		return false;
	}

	const TArray<AActor*> ActorsToIgnore;
	static const float DrawTimeSeconds = 0.f;

	const bool bReturnedHit = UKismetSystemLibrary::LineTraceSingle(
		World,
		StartWorld,
		EndWorld,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		OutHit,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		DrawTimeSeconds);

	return true == bReturnedHit && true == OutHit.bBlockingHit;
}

void ACMagicWarpSkillContext::Warp_TryResolveAiWarpLocation(FVector& OutLocation, bool& bOutOk) const
{
	OutLocation = FVector::ZeroVector;
	bOutOk = false;

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicWarp] Warp_TryResolveAiWarpLocation: Character 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	APawn* asPawn = Cast<APawn>(Character);
	if (false == IsValid(asPawn))
	{
		return;
	}

	AController* ctlResolved = asPawn->GetController();
	AAIController* aiCtl = Cast<AAIController>(ctlResolved);
	if (false == IsValid(aiCtl))
	{
		// 블루프린트 Cast 실패 브랜치 무연결과 동등 — 플레이어 PC 에서 빈번.
		return;
	}

	UBlackboardComponent* bbResolved = UAIBlueprintHelperLibrary::GetBlackboard(aiCtl);
	if (false == IsValid(bbResolved))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicWarp] AIController 블랙보드 없음 — EQ_Location 조회 불가 — %s"),
			*GetNameSafe(this)));
		return;
	}

	OutLocation = bbResolved->GetValueAsVector(WarpBlackboardEqLocationKey);
	bOutOk = true;
}

void ACMagicWarpSkillContext::Warp_EndActionAndUnequipIfBusy()
{
	if (false == Magic_IsOwnerInAction())
	{
		return;
	}

	End_DoAction(CEAttackType::Common);
	Unequip();
}

void ACMagicWarpSkillContext::GetCursorLocationAndRotation(
	bool& OutHit,
	FVector& OutLocation,
	FRotator& OutRotation)
{
	OutHit = false;
	OutLocation = FVector::ZeroVector;
	OutRotation = FRotator::ZeroRotator;

	APlayerController* pcResolved = Warp_ResolvePlayerController();
	if (false == IsValid(pcResolved))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicWarp] GetCursorLocationAndRotation: PlayerController 없음 — %s"),
			*GetNameSafe(this)));
		return;
	}

	FHitResult Hit;
	const bool bGotHit = pcResolved->GetHitResultUnderCursorByChannel(
		ETraceTypeQuery::TraceTypeQuery1,
		true,
		Hit);

	if (bGotHit && Hit.bBlockingHit)
	{
		OutHit = true;
		OutLocation = Hit.Location;
		OutRotation = UKismetMathLibrary::Conv_VectorToRotator(Hit.ImpactNormal);
	}
}

void ACMagicWarpSkillContext::SetCandidate(const FVector& InLocation)
{
	WarpCandidateLocation = InLocation;

	const bool bRealRiding = Magic_IsCharacterRealRiding();
	if (bRealRiding)
	{
		PlayAction(DoActionDatas, 0);
		return;
	}

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[MagicWarp] SetCandidate: Character 없어 회전 생략 — PlayAction(0)만 시도 — %s"),
			*GetNameSafe(this)));
		PlayAction(DoActionDatas, 0);
		return;
	}

	const FVector Start = Character->GetActorLocation();
	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(Start, InLocation);
	const FRotator YawOnly(0.f, LookAt.Yaw, 0.f);
	Character->SetActorRotation(YawOnly, ETeleportType::None);

	PlayAction(DoActionDatas, 0);
}
