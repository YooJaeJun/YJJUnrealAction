#include "Components/CFeetComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include <limits>

namespace
{
/** 양 발 법선(Z-up 표면 노멀)으로 지면 기울기 — FeetComponent.Trace 와 같은 DegAtan2 조합이다. */
FRotator GroundRotationFromSurfaceNormalXYZ(const FVector& Normal)
{
	const FVector n = Normal.GetSafeNormal();
	const float rollDeg = UKismetMathLibrary::DegAtan2(static_cast<float>(n.Y), static_cast<float>(n.Z));
	const float atanXZ = UKismetMathLibrary::DegAtan2(static_cast<float>(n.X), static_cast<float>(n.Z));
	const float pitchDeg = -1.f * atanXZ;
	return FRotator(pitchDeg, 0.f, rollDeg);
}
} // namespace

UCFeetComponent::UCFeetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCFeetComponent::BeginPlay()
{
	Super::BeginPlay();

	// BP FeetComponent.Character: BlueprintPrivate 캐시 — 미지정이면 폰 소유 컴포넌트 규약으로 동일 처리.
	if (false == IsValid(Character))
	{
		Character = Cast<ACharacter>(GetOwner());
	}
}

void UCFeetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ApplyInverseKinematics(static_cast<double>(DeltaTime));
}

void UCFeetComponent::TraceGroundFromSocketBP(
	FName InSocketName,
	USkeletalMeshComponent* InSkeletalMesh,
	double& OutDistance,
	FRotator& OutRotation,
	FVector& OutImpactPoint)
{
	OutDistance = 0.0;
	OutRotation = FRotator::ZeroRotator;
	OutImpactPoint = FVector::ZeroVector;

	ACharacter* TraceCharacter = Character.Get();
	if (false == IsValid(TraceCharacter))
	{
		TraceCharacter = Cast<ACharacter>(GetOwner());
	}

	if (false == IsValid(TraceCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("FeetComponent.Trace: Character 가 유효하지 않습니다."));
		return;
	}

	if (false == IsValid(InSkeletalMesh))
	{
		UE_LOG(LogTemp, Warning, TEXT("FeetComponent.Trace: InSkeletalMesh 가 없습니다."));
		return;
	}

	if (InSocketName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("FeetComponent.Trace: 소켓 이름이 None 입니다."));
		return;
	}

	UCapsuleComponent* capsule = TraceCharacter->GetCapsuleComponent();
	if (false == IsValid(capsule))
	{
		UE_LOG(LogTemp, Warning, TEXT("FeetComponent.Trace: CapsuleComponent 가 없습니다."));
		return;
	}

	const float halfH = capsule->GetScaledCapsuleHalfHeight();
	FHitResult hit;
	double raw = 0.0;
	FRotator rot = FRotator::ZeroRotator;
	const bool bHit = TraceLimbGroundMatchBPTrace(TraceCharacter, InSkeletalMesh, InSocketName, halfH, hit, raw, rot);
	if (false != bHit)
	{
		OutDistance = raw;
		OutRotation = rot;
		OutImpactPoint = hit.ImpactPoint;
	}
}

bool UCFeetComponent::TraceFourLegGroundBP(FVector InWorldTraceStart, FVector& OutHitLocation, FVector& OutHitNormal)
{
	// BP Trace4Leg: End = In + (0,0,-100), bTraceComplex=false, ActorsToIgnore 비어 있음, DrawDebug 기본 None.
	const FVector traceEnd = InWorldTraceStart + FVector(0.f, 0.f, -100.f);
	TArray<AActor*> emptyIgnores;
	FHitResult hit;
	const bool bHit = UKismetSystemLibrary::LineTraceSingle(
		this,
		InWorldTraceStart,
		traceEnd,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		emptyIgnores,
		EDrawDebugTrace::None,
		hit,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.f);

	if (false != bHit && hit.IsValidBlockingHit())
	{
		OutHitLocation = hit.Location;
		OutHitNormal = hit.ImpactNormal;
		return true;
	}

	OutHitLocation = InWorldTraceStart;
	OutHitNormal = FVector::ZeroVector;
	return false;
}

USkeletalMeshComponent* UCFeetComponent::ResolveTargetMesh() const
{
	if (IsValid(TargetMesh))
		return TargetMesh;

	ACharacter* ch = Character.Get();
	if (IsValid(ch) && IsValid(ch->GetMesh()))
		return ch->GetMesh();

	AActor* Ow = GetOwner();
	ACharacter* castedOwner = Cast<ACharacter>(Ow);
	if (castedOwner && IsValid(castedOwner->GetMesh()))
		return castedOwner->GetMesh();

	return nullptr;
}

bool UCFeetComponent::TraceLimbGroundMatchBPTrace(
	ACharacter* TraceCharacter,
	USkeletalMeshComponent* Mesh,
	FName SocketName,
	float CapsuleScaledHalfHeight,
	FHitResult& OutHit,
	double& OutRawDistanceSigned,
	FRotator& OutGroundRotation_FromHitNormal_NormalComponent) const
{
	OutRawDistanceSigned = 0.0;
	OutGroundRotation_FromHitNormal_NormalComponent = FRotator::ZeroRotator;
	OutHit = FHitResult();

	UWorld* world = GetWorld();
	if ((false == IsValid(TraceCharacter)) || (false == IsValid(Mesh)) || (false == IsValid(world)))
		return false;

	const FVector socketLoc = Mesh->GetSocketLocation(SocketName);
	const FVector actorLoc = TraceCharacter->GetActorLocation();

	// FeetComponent.Trace 로컬 변수 Start — 소켓 XY, 폰 월드 Z.
	const FVector traceStart(socketLoc.X, socketLoc.Y, actorLoc.Z);
	const float traceLen = static_cast<float>(TraceDistance);
	const float capsuleHalfScaled = CapsuleScaledHalfHeight;
	const float endZFloat = traceStart.Z - capsuleHalfScaled - traceLen;
	const FVector traceEnd(socketLoc.X, socketLoc.Y, endZFloat);

	TArray<AActor*> ignoresActors;
	ignoresActors.Add(TraceCharacter);

	UKismetSystemLibrary::LineTraceSingle(
		this,
		traceStart,
		traceEnd,
		ETraceTypeQuery::TraceTypeQuery1,
		true,
		ignoresActors,
		DrawDebug,
		OutHit,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.f);

	if (false == OutHit.IsValidBlockingHit())
		return false;

	// BP Distance: Size(ImpactPoint - TraceEnd) + OffsetDistance - TraceDistance
	const float residualLen = FVector::Dist(OutHit.ImpactPoint, OutHit.TraceEnd);
	const float offsetF = static_cast<float>(OffsetDistance);
	OutRawDistanceSigned = static_cast<double>(residualLen + offsetF - traceLen);

	// BP Break Hit 의 Normal 피 노드로 MakeRotator( Roll = Atan2(N.Y,N.Z), Pitch = -(Atan2(N.X,N.Z)), … ).
	OutGroundRotation_FromHitNormal_NormalComponent = GroundRotationFromSurfaceNormalXYZ(OutHit.Normal);
	return true;
}

void UCFeetComponent::ApplyInverseKinematics(double InDeltaSeconds)
{
	if (InDeltaSeconds <= 0.0)
		return;

	ACharacter* ch = Character.Get();
	if (false == IsValid(ch))
		ch = Cast<ACharacter>(GetOwner());

	if (false == IsValid(ch))
		return;

	UCapsuleComponent* capsule = ch->GetCapsuleComponent();
	USkeletalMeshComponent* mesh = ResolveTargetMesh();
	if ((false == IsValid(mesh)) || (false == IsValid(capsule)))
	{
		return;
	}

	const float Dt = static_cast<float>(InDeltaSeconds);
	const float spd = static_cast<float>(InterpSpeed);
	const float halfH = capsule->GetScaledCapsuleHalfHeight();

	FHitResult hLf, hRf, hLh, hRh;
	double lfRaw = 0.0, rfRaw = 0.0, lhRaw = 0.0, rhRaw = 0.0;
	FRotator rotLf = FRotator::ZeroRotator, rotRf = FRotator::ZeroRotator, rotLh = FRotator::ZeroRotator, rotRh = FRotator::ZeroRotator;
	const bool bLf = TraceLimbGroundMatchBPTrace(ch, mesh, Point_1, halfH, hLf, lfRaw, rotLf);
	const bool bRf = TraceLimbGroundMatchBPTrace(ch, mesh, Point_2, halfH, hRf, rfRaw, rotRf);
	const bool bLh = TraceLimbGroundMatchBPTrace(ch, mesh, Point_3, halfH, hLh, lhRaw, rotLh);
	const bool bRh = TraceLimbGroundMatchBPTrace(ch, mesh, Point_4, halfH, hRh, rhRaw, rotRh);

	// 레거시 2발: 골반 보정 높이 = 발 트레이스 중 더 낮게 잡히는 값. 4발은 손 포함 최소 깊이로 동일 패턴 확장한다.
	double pelvisBaseline = std::numeric_limits<double>::max();
	bool bBaselineFromHit = false;
	if (false != bLf)
	{
		pelvisBaseline = FMath::Min(pelvisBaseline, lfRaw);
		bBaselineFromHit = true;
	}
	if (false != bRf)
	{
		pelvisBaseline = FMath::Min(pelvisBaseline, rfRaw);
		bBaselineFromHit = true;
	}
	if (false != bLh)
	{
		pelvisBaseline = FMath::Min(pelvisBaseline, lhRaw);
		bBaselineFromHit = true;
	}
	if (false != bRh)
	{
		pelvisBaseline = FMath::Min(pelvisBaseline, rhRaw);
		bBaselineFromHit = true;
	}
	if (false == bBaselineFromHit)
		pelvisBaseline = 0.0;

	// 레거시 YJJActionCpp: 골반 Z 는 offset, 발·손은 상대적으로 ±X — Data(FVector) 에 동일 패턴 적용한다.
	Data.PelvisDistance.Z =
		UKismetMathLibrary::FInterpTo(Data.PelvisDistance.Z, static_cast<float>(pelvisBaseline), Dt, spd);

	const float targLfX =
		false != bLf ? static_cast<float>(lfRaw - pelvisBaseline) : 0.f;
	const float targRfX =
		false != bRf ? -static_cast<float>(rfRaw - pelvisBaseline) : 0.f;
	const float targLhX =
		false != bLh ? static_cast<float>(lhRaw - pelvisBaseline) : 0.f;
	const float targRhX =
		false != bRh ? -static_cast<float>(rhRaw - pelvisBaseline) : 0.f;

	Data.LeftFootDistance.X =
		UKismetMathLibrary::FInterpTo(static_cast<float>(Data.LeftFootDistance.X), targLfX, Dt, spd);
	Data.RightFootDistance.X =
		UKismetMathLibrary::FInterpTo(static_cast<float>(Data.RightFootDistance.X), targRfX, Dt, spd);
	Data.LeftHandDistance.X =
		UKismetMathLibrary::FInterpTo(static_cast<float>(Data.LeftHandDistance.X), targLhX, Dt, spd);
	Data.RightHandDistance.X =
		UKismetMathLibrary::FInterpTo(static_cast<float>(Data.RightHandDistance.X), targRhX, Dt, spd);

	Data.LeftFootRotation = UKismetMathLibrary::RInterpTo(Data.LeftFootRotation, false != bLf ? rotLf : FRotator::ZeroRotator, Dt, spd);
	Data.RightFootRotation = UKismetMathLibrary::RInterpTo(Data.RightFootRotation, false != bRf ? rotRf : FRotator::ZeroRotator, Dt, spd);
	Data.LeftHandRotation = UKismetMathLibrary::RInterpTo(Data.LeftHandRotation, false != bLh ? rotLh : FRotator::ZeroRotator, Dt, spd);
	Data.RightHandRotation = UKismetMathLibrary::RInterpTo(Data.RightHandRotation, false != bRh ? rotRh : FRotator::ZeroRotator, Dt, spd);

	LeftFootRotation = Data.LeftFootRotation;
	RightFootRotation = Data.RightFootRotation;
	LeftHandRotation = Data.LeftHandRotation;
	RightHandRotation = Data.RightHandRotation;

	LeftFootDistance_Save =
		static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(LeftFootDistance_Save), static_cast<float>(lfRaw), Dt, spd));
	LeftFootDistance =
		static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(LeftFootDistance), static_cast<float>(LeftFootDistance_Save), Dt, spd));

	RightFootDistance_Save =
		static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(RightFootDistance_Save), static_cast<float>(rfRaw), Dt, spd));
	RightFootDistance =
		static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(RightFootDistance), static_cast<float>(RightFootDistance_Save), Dt, spd));

	LeftHandDistance_Save =
		static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(LeftHandDistance_Save), static_cast<float>(lhRaw), Dt, spd));
	LeftHandDistance =
		static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(LeftHandDistance), static_cast<float>(LeftHandDistance_Save), Dt, spd));

	RightHandDistance_Save =
		static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(RightHandDistance_Save), static_cast<float>(rhRaw), Dt, spd));
	RightHandDistance =
		static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(RightHandDistance), static_cast<float>(RightHandDistance_Save), Dt, spd));

	PelvisDistance_Save =
		static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(PelvisDistance_Save), static_cast<float>(pelvisBaseline), Dt, spd));
	PelvisDistance =
		static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(PelvisDistance), static_cast<float>(PelvisDistance_Save), Dt, spd));

	LeftFoot = static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(LeftFoot), static_cast<float>(lfRaw), Dt, spd));
	RightFoot = static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(RightFoot), static_cast<float>(rfRaw), Dt, spd));
	LeftHand = static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(LeftHand), static_cast<float>(lhRaw), Dt, spd));
	RightHand = static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(RightHand), static_cast<float>(rhRaw), Dt, spd));
	Pelvis = static_cast<double>(UKismetMathLibrary::FInterpTo(static_cast<float>(Pelvis), static_cast<float>(pelvisBaseline), Dt, spd));

	// FeetComponent.Trace 는 BreakHit 의 Normal 로 기울기를 만든다 — GroundSlope 블렌드에 동일 피 채택.
	FVector normalAvg = FVector::UpVector;
	const int32 feetHitCount = static_cast<int32>(bLf) + static_cast<int32>(bRf);
	if (2 == feetHitCount)
		normalAvg = (hLf.Normal + hRf.Normal).GetSafeNormal();
	else if (false != bLf)
		normalAvg = hLf.Normal;
	else if (false != bRf)
		normalAvg = hRf.Normal;

	const FRotator slopeTarget = GroundRotationFromSurfaceNormalXYZ(normalAvg);
	GroundSlope = UKismetMathLibrary::RInterpTo(GroundSlope, slopeTarget, Dt, spd);
	PelvisRotation = UKismetMathLibrary::RInterpTo(PelvisRotation, slopeTarget, Dt, spd);
	Data.PelvisRotation = PelvisRotation;
}
