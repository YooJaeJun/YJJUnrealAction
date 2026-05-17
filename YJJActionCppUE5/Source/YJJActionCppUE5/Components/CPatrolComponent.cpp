#include "Components/CPatrolComponent.h"
#include "Components/SplineComponent.h"
#include "UObject/UnrealType.h"

UCPatrolComponent::UCPatrolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPatrolComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UCPatrolComponent::IsValidPath() const
{
	return IsValid(Path.Get());
}

bool UCPatrolComponent::TryReadBlueprintBoolProperty(UObject* Object, const FName PropertyName, bool& OutValue)
{
	OutValue = false;
	if (false == IsValid(Object))
	{
		return false;
	}

	UClass* const ObjClass = Object->GetClass();
	if (nullptr == ObjClass)
	{
		return false;
	}

	const FBoolProperty* const BoolProp = CastField<FBoolProperty>(ObjClass->FindPropertyByName(PropertyName));
	if (nullptr == BoolProp)
	{
		// 레거시 BP 변수명 아님이면 루프는 false 처리(설계 검증 목적 디테일 블루프린트 쪽 확인).
		return false;
	}

	const void* const ValueAddr = BoolProp->ContainerPtrToValuePtr<void>(Object);
	if (nullptr == ValueAddr)
	{
		return false;
	}

	OutValue = BoolProp->GetPropertyValue(ValueAddr);
	return true;
}

USplineComponent* UCPatrolComponent::ResolveSpline(AActor* PathActor) const
{
	if (false == IsValid(PathActor))
	{
		return nullptr;
	}

	// BP_PatrolPass::Spline 과 동등 — 컴포넌트 이름이 분리되어 있어도 보통 클래스 단일 검색으로 충분.
	return PathActor->FindComponentByClass<USplineComponent>();
}

void UCPatrolComponent::GetMoveTo(bool& OutResult, FVector& OutLocation)
{
	OutResult = false;
	OutLocation = FVector::ZeroVector;

	if (false == IsValidPath())
	{
		return;
	}

	AActor* const PathActor = Path.Get();
	USplineComponent* const Spline = ResolveSpline(PathActor);
	if (nullptr == Spline)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[PatrolComponent] '%s': Path 에 USplineComponent 가 없음 — BP_PatrolPass 등 확인."),
			*GetNameSafe(GetOwner()));
		return;
	}

	const int32 NumPoints = Spline->GetNumberOfSplinePoints();
	if (NumPoints <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PatrolComponent] '%s': 스플라인 포인트가 0."), *GetNameSafe(GetOwner()));
		return;
	}

	// BP 는 Raw Index 를 넘김 — 엔진 GetLocation 범위 초과 방지만 최소 Clamp.
	const int32 ClampedIndex = FMath::Clamp(Index, 0, NumPoints - 1);
	if (ClampedIndex != Index)
	{
		UE_LOG(
			LogTemp,
			Verbose,
			TEXT("[PatrolComponent] '%s': Index 가 스플라인 범위를 벗어나 Clamp 함 (Index=%d, NumPoints=%d)."),
			*GetNameSafe(GetOwner()),
			Index,
			NumPoints);
	}

	OutLocation = Spline->GetLocationAtSplinePoint(ClampedIndex, ESplineCoordinateSpace::World);
	OutResult = true;
}

void UCPatrolComponent::UpdateNextIndex()
{
	if (false == IsValidPath())
	{
		return;
	}

	AActor* const PathActor = Path.Get();
	if (nullptr == PathActor)
	{
		return;
	}

	USplineComponent* const Spline = ResolveSpline(PathActor);
	if (nullptr == Spline)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[PatrolComponent] '%s': UpdateNextIndex 에서 Spline 미해결."), *GetNameSafe(GetOwner()));
		return;
	}

	const int32 Count = Spline->GetNumberOfSplinePoints();

	// 0·1 포인트는 순찰 패턴 불명 — 업데이트 스킵(레거시 BT 가 먼저 IsValidPath 를 쓰도록 기대).
	if (Count <= 1)
	{
		return;
	}

	bool bLoop = false;
	(void)TryReadBlueprintBoolProperty(PathActor, FName(TEXT("Loop")), bLoop);

	// Reverse == true 브랜치(역방향)
	if (Reverse)
	{
		if (Index > 0)
		{
			--Index;
			return;
		}

		if (bLoop)
		{
			Index = Count - 1;
			return;
		}

		Index = 1;
		Reverse = false;
		return;
	}

	// Reverse == false(정방향)
	if (Index < Count - 1)
	{
		++Index;
		return;
	}

	if (bLoop)
	{
		Index = 0;
		return;
	}

	Index = Count - 2;
	Reverse = true;
}
