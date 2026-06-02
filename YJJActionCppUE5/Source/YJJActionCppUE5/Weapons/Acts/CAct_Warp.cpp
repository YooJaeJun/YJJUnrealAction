#include "Weapons/Acts/CAct_Warp.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Components/CStateComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "Weapons/CAttachment.h"
#include "Weapons/CEquipment.h"

UCAct_Warp::UCAct_Warp()
{
}

void UCAct_Warp::BeginPlay(
	TWeakObjectPtr<ACCommonCharacter> InOwner,
	TWeakObjectPtr<ACAttachment> InAttachment,
	TWeakObjectPtr<UCEquipment> InEquipment,
	const TArray<FDoActionData>& InActData, 
	const TArray<FHitData>& InHitData)
{
	Super::BeginPlay(InOwner, InAttachment, InEquipment, InActData, InHitData);

	if (InAttachment.IsValid())
	{
		Decal = YJJHelpers::GetComponent<UDecalComponent>(InAttachment.Get());
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[Weapon][Act_Warp] Attachment 없음 — Decal 미바인드 Owner=%s"),
			InOwner.IsValid() ? *InOwner->GetName() : TEXT("(null)")));
	}

	if (InOwner.IsValid())
		Controller = InOwner->GetController();
}

void UCAct_Warp::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);

	if (false == Decal.IsValid())
		return;

	FVector location = FVector::ZeroVector;
	FRotator rotation = FRotator::ZeroRotator;

	if (false == GetCursorLocationAndRotation(location, rotation))
	{
		Decal->SetVisibility(false);
		return;
	}

	if (bInAct)
		return;

	Decal->SetVisibility(true);

	Decal->SetWorldLocation(location);
	Decal->SetWorldRotation(rotation);
}

void UCAct_Warp::Act()
{
	CheckFalse(ActDatas.Num() > 0);
	CheckFalse(StateComp->IsIdleMode());

	Super::Act();

	FRotator rotation;

	const bool bValid = GetCursorLocationAndRotation(MoveToLocation, rotation);
	CheckFalse(bValid);

	const float height = Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	MoveToLocation = FVector(MoveToLocation.X, MoveToLocation.Y, MoveToLocation.Z + height);

	const float yaw =
		UKismetMathLibrary::FindLookAtRotation(
			Owner->GetActorLocation(), MoveToLocation).Yaw;
	Owner->SetActorRotation(FRotator(0, yaw, 0));

	ActDatas[0].Act(Owner);
}

void UCAct_Warp::Begin_Act()
{
	Super::Begin_Act();

	Owner->SetActorLocation(MoveToLocation);
	MoveToLocation = FVector::ZeroVector;
}

bool UCAct_Warp::GetCursorLocationAndRotation(FVector& OutLocation, FRotator& OutRotation) const
{
	CheckNullResult(Controller, false);

	FHitResult hitResult;

	// 커서 아래 월드 히트는 PlayerController API 이므로 로컬 조작자 전제로 캐스트한다.
	const TWeakObjectPtr<APlayerController> playerController =
		Cast<APlayerController>(Controller);

	playerController->GetHitResultUnderCursorByChannel(
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		hitResult);

	// TODO: 최대 워프 거리·내비 가능 지점·태그/레이어 제외 등 규칙 검증을 추가할 수 있다.

	CheckFalseResult(hitResult.bBlockingHit, false);

	OutLocation = hitResult.Location;
	OutRotation = hitResult.ImpactNormal.Rotation();

	return true;
}
