#include "Components/CTargetingComponent.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "CStateComponent.h"
#include "CMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/HitResult.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Particles/ParticleSystemComponent.h"

namespace
{
	TWeakObjectPtr<AController> ResolveTargetingController(ACCommonCharacter* InOwnerCharacter)
	{
		if (!IsValid(InOwnerCharacter))
		{
			return nullptr;
		}

		TWeakObjectPtr<AController> outController = InOwnerCharacter->GetMyCurController();
		if (outController.IsValid())
		{
			return outController;
		}

		return InOwnerCharacter->GetController();
	}
}

void UCTargetingComponent::FixCharacterCamera()
{
	if (false == OwnerCharacter.IsValid())
		return;

	const TObjectPtr<UCMovementComponent> movingLocal =
		YJJHelpers::GetComponent<UCMovementComponent>(OwnerCharacter.Get());
	if (IsValid(movingLocal))
		movingLocal->FixCamera();
}

void UCTargetingComponent::UnFixCharacterCamera()
{
	if (false == OwnerCharacter.IsValid())
		return;

	const TObjectPtr<UCMovementComponent> movingLocal =
		YJJHelpers::GetComponent<UCMovementComponent>(OwnerCharacter.Get());
	if (IsValid(movingLocal))
		movingLocal->UnFixCamera();
}

void UCTargetingComponent::DestroyMarkerParticle()
{
	if (IsValid(Particle))
	{
		Particle->DestroyComponent();
		Particle = nullptr;
	}
}

void UCTargetingComponent::SpawnMarkerParticleForTarget(ACCommonCharacter* InTarget)
{
	if (!IsValid(InTarget))
	{
		CLog::Log(TEXT("Targeting: SpawnMarkerParticleForTarget 호출 시 InTarget 가 무효합니다."));
		return;
	}

	if (false == IsValid(ParticleAsset))
	{
		CLog::Log(FString::Printf(TEXT("Targeting: ParticleAsset 미설정 — 대상=%s"),
			*InTarget->GetName()));
		return;
	}

	const TObjectPtr<USkeletalMeshComponent> meshLocal = InTarget->GetMesh();
	if (false == IsValid(meshLocal))
	{
		CLog::Log(FString::Printf(TEXT("Targeting: 대상에게 SkeletalMesh 가 없음 — %s"),
			*InTarget->GetName()));
		return;
	}

	DestroyMarkerParticle();

	Particle = UGameplayStatics::SpawnEmitterAttached(
		ParticleAsset,
		meshLocal,
		FName(TEXT("Root")),
		FVector(0.0f, 0.0f, -10.0f),
		FRotator::ZeroRotator,
		FVector(0.25f, 0.25f, 0.25f),
		EAttachLocation::KeepRelativeOffset,
		true,
		EPSCPoolMethod::None,
		true);
}

UCTargetingComponent::UCTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCTargetingComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACCommonCharacter>(GetOwner());
	if (!OwnerCharacter.IsValid())
	{
		CLog::Log(FString(TEXT("TargetingComponent 의 Owner 가 ACCommonCharacter 가 아니어서 타게팅을 초기화할 수 없습니다.")));
		return;
	}

	MovingFocusCurCoolTime = MovingFocusInitTime;
	Controller = ResolveTargetingController(OwnerCharacter.Get());
}

void UCTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Tick_MoveFocusCoolTIme(DeltaTime);
	Tick_Targeting();
}

void UCTargetingComponent::InputAction_Targeting()
{
	// 레거시 BP Toggle_Target 분기 — `bTargeting` 이 아니라 잠금 대상 레퍼런스 존재 여부로 분기했다.
	if (Target.IsValid())
		End_Targeting();
	else
		Begin_Targeting();
}

void UCTargetingComponent::Begin_Targeting()
{
	CheckNull(OwnerCharacter.Get());

	const TArray<AActor*> ignores{ OwnerCharacter.Get() };
	TArray<FHitResult> hitResults;

	UKismetSystemLibrary::SphereTraceMultiByProfile(GetWorld(),
		OwnerCharacter->GetActorLocation(),
		OwnerCharacter->GetActorLocation(),
		TraceDistance,
		TEXT("Targeting"),
		false,
		ignores,
		DrawDebug,
		hitResults,
		true);

	TArray<TWeakObjectPtr<ACCommonCharacter>> targets;

	for (const FHitResult& elem : hitResults)
	{
		AActor* const hitActor = elem.GetActor();
		if (!IsValid(hitActor))
			continue;

		ACCommonCharacter* const asCharacter = Cast<ACCommonCharacter>(hitActor);
		if (!IsValid(asCharacter))
			continue;

		if (hitActor->GetClass() == OwnerCharacter->GetClass())
			continue;

		targets.AddUnique(asCharacter);
	}

	const TWeakObjectPtr<AController> ownerControllerWeak = ResolveTargetingController(OwnerCharacter.Get());
	const TWeakObjectPtr<ACCommonCharacter> target =
		YJJHelpers::GetNearForward(OwnerCharacter, targets, ownerControllerWeak);

	ChangeTarget(target.Get());
}

void UCTargetingComponent::End_Targeting()
{
	// 레거시 BP 순서: UI 숨김 → 이동 컴포넌트 카메라 해제 → Target 비움 → 마커 제거 → bTargeting=false
	if (Target.IsValid())
	{
		SetVisibleTargetUI(false);
	}

	UnFixCharacterCamera();

	Target.Reset();
	TargetStateComp.Reset();
	TargetMovementComp.Reset();
	TargetingWidgetComp.Reset();

	DestroyMarkerParticle();

	bTargeting = false;
}

void UCTargetingComponent::ChangeTarget(ACCommonCharacter* InTarget)
{
	if (IsValid(InTarget))
	{
		if (Target.IsValid())
			SetVisibleTargetUI(false);

		Target = InTarget;

		TargetStateComp = YJJHelpers::GetComponent<UCStateComponent>(Target.Get());
		TargetMovementComp = YJJHelpers::GetComponent<UCMovementComponent>(Target.Get());

		UWidgetComponent* const widgetFromTarget =
			YJJHelpers::GetComponent<UWidgetComponent>(Target.Get());
		if (IsValid(widgetFromTarget))
		{
			TargetingWidgetComp = widgetFromTarget;
		}
		else
		{
			TargetingWidgetComp.Reset();
		}

		SpawnMarkerParticleForTarget(InTarget);

		SetVisibleTargetUI(true);

		FixCharacterCamera();

		bTargeting = true;
	}
	else
	{
		End_Targeting();
	}
}

void UCTargetingComponent::SetVisibleTargetUI(bool bVisible)
{
	if (!Target.IsValid())
		return;

	UWidgetComponent* widget = TargetingWidgetComp.Get();
	if (!IsValid(widget))
	{
		widget = YJJHelpers::GetComponent<UWidgetComponent>(Target.Get());
		if (IsValid(widget))
		{
			TargetingWidgetComp = widget;
		}
	}

	if (!IsValid(widget))
	{
		CLog::Log(FString::Printf(TEXT("Targeting: SetVisibleTargetUI 실패 — UWidgetComponent 없음 — Target=%s"),
			*Target->GetName()));
		return;
	}

	widget->SetVisibility(bVisible);
}

void UCTargetingComponent::Tick_MoveFocusCoolTIme(const float InDelta)
{
	// BP Tick_MoveFocusCoolTIme: CanMoveFocus 가 true 일 때는 쿨다운 차감 없음(false 분기만 누적).
	if (bCanMoveFocus)
		return;

	MovingFocusCurCoolTime -= static_cast<float>(InDelta);

	if (MovingFocusCurCoolTime < 0.0f)
	{
		MovingFocusCurCoolTime = MovingFocusInitTime;
		bCanMoveFocus = true;
	}
}

void UCTargetingComponent::Tick_Targeting()
{
	if (!OwnerCharacter.IsValid() || !Target.IsValid())
		return;

	if (!TargetStateComp.IsValid())
		TargetStateComp = YJJHelpers::GetComponent<UCStateComponent>(Target.Get());

	if (!TargetStateComp.IsValid())
	{
		CLog::Log(FString(TEXT("Targeting: 대상에게 UCStateComponent 가 없어 타게팅을 종료했습니다.")));
		End_Targeting();
		return;
	}

	bool bIsValid = true;
	bIsValid &= (false == TargetStateComp->IsDeadMode());
	bIsValid &= (true == (OwnerCharacter->GetDistanceTo(Target.Get()) <= TraceDistance));

	if (false == bIsValid)
	{
		End_Targeting();
		return;
	}

	if (!Controller.IsValid())
		Controller = ResolveTargetingController(OwnerCharacter.Get());

	// 레거시 BP Tick_Targeting: 매 틱 GetMyCurController 이 비면 BP_Player 의 CurController 로 보정.
	CheckFalse(Controller.IsValid());

	const FRotator controlRotation = Controller->GetControlRotation();
	const FRotator ownerToTarget = UKismetMathLibrary::FindLookAtRotation(
		OwnerCharacter->GetActorLocation(), Target->GetActorLocation());

	if (true == UKismetMathLibrary::EqualEqual_RotatorRotator(controlRotation, ownerToTarget, FinishAngle))
	{
		Controller->SetControlRotation(ownerToTarget);

		if (true == bMovingFocus)
			bMovingFocus = false;
	}
	else
	{
		const FRotator rotator = UKismetMathLibrary::RInterpTo(controlRotation,
			ownerToTarget,
			static_cast<float>(UGameplayStatics::GetWorldDeltaSeconds(GetWorld())),
			static_cast<float>(InterpSpeed));

		Controller->SetControlRotation(rotator);
	}
}

void UCTargetingComponent::ChangeFocus(const bool InRight)
{
	CheckNull(OwnerCharacter.Get());

	CheckFalse(bCanMoveFocus);
	CheckFalse(Target.IsValid());

	const TArray<AActor*> ignores{ OwnerCharacter.Get(), Target.Get() };
	TArray<FHitResult> hitResults;

	UKismetSystemLibrary::SphereTraceMultiByProfile(GetWorld(),
		OwnerCharacter->GetActorLocation(),
		OwnerCharacter->GetActorLocation(),
		TraceDistance,
		TEXT("Targeting"),
		false,
		ignores,
		DrawDebug,
		hitResults,
		true);

	TArray<TWeakObjectPtr<ACCommonCharacter>> targets;

	for (const FHitResult& elem : hitResults)
	{
		AActor* const hitActor = elem.GetActor();
		if (!IsValid(hitActor))
			continue;

		if (hitActor->GetClass() == OwnerCharacter->GetClass())
			continue;

		ACCommonCharacter* const asCharacter = Cast<ACCommonCharacter>(hitActor);
		if (!IsValid(asCharacter))
			continue;

		targets.AddUnique(asCharacter);
	}

	TMap<float, TWeakObjectPtr<ACCommonCharacter>> nearCharacters;

	const TWeakObjectPtr<AController> ownerControllerWeak = ResolveTargetingController(OwnerCharacter.Get());
	YJJHelpers::AddNearSideCharacters(OwnerCharacter, targets, ownerControllerWeak,
		nearCharacters);

	float minAngle = 1e9f;

	TWeakObjectPtr<ACCommonCharacter> candidate;

	for (const TPair<float, TWeakObjectPtr<ACCommonCharacter>>& elem : nearCharacters)
	{
		const float curAngle = elem.Key;

		bool bCheck = false;
		bCheck |= (InRight && curAngle > 0.0f);
		bCheck |= (false == InRight && curAngle < 0.0f);
		bCheck &= (minAngle > FMath::Abs(curAngle));

		if (bCheck)
		{
			minAngle = FMath::Abs(curAngle);
			candidate = elem.Value;
		}
	}

	if (candidate.IsValid())
	{
		if (false == bMovingFocus)
		{
			bMovingFocus = true;
			ChangeTarget(candidate.Get());
		}
	}

	bCanMoveFocus = false;
}
