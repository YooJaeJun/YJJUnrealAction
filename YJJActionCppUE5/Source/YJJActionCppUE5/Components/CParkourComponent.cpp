#include "Components/CParkourComponent.h"
#include "Global.h"
#include "Components/ArrowComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CStateComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	FName ResolveArrowActorSubobjectName(const CEParkourArrowType InType)
	{
		switch (InType)
		{
		case CEParkourArrowType::Ceil:
			return FName(TEXT("Ceil"));
		case CEParkourArrowType::Center:
			return FName(TEXT("Center"));
		case CEParkourArrowType::Floor:
			return FName(TEXT("Floor"));
		case CEParkourArrowType::Land:
			return FName(TEXT("Land_0"));
		case CEParkourArrowType::Left:
			return FName(TEXT("Left"));
		case CEParkourArrowType::Right:
			return FName(TEXT("Right"));
		case CEParkourArrowType::Max:
		default:
			return NAME_None;
		}
	}

	bool IsNearlyZeroSkipLogVector2D_XY(const FVector& InValue, const float Epsilon = 2.f)
	{
		const double XValue = static_cast<double>(InValue.X);
		const double YValue = static_cast<double>(InValue.Y);
		return XValue * XValue + YValue * YValue < static_cast<double>(Epsilon) * static_cast<double>(Epsilon);
	}

	bool ParkourYawWithinFrontDegrees(
		const FVector& ActorForwardWorld,
		const FVector& ReferenceWorld,
		const double AllowedDegrees)
	{
		const FVector FN = FVector::VectorPlaneProject(ActorForwardWorld, FVector::UpVector).GetSafeNormal();
		FVector DN = FVector::VectorPlaneProject(ReferenceWorld, FVector::UpVector);
		DN.Normalize(KINDA_SMALL_NUMBER);
		if (DN.IsNearlyZero())
			return false;

		const double DotClamp = static_cast<double>(FMath::Clamp(FVector::DotProduct(FN, DN), -1.0f, 1.0f));
		const double Rad = FMath::Acos(DotClamp);
		const double Deg = FMath::RadiansToDegrees(Rad);
		return Deg <= AllowedDegrees;
	}
}

UCParkourComponent::UCParkourComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UDataTable> ParkourDataAsset(
		TEXT("/Game/Parkour/ParkourData.ParkourData"));
	if (ParkourDataAsset.Succeeded())
		DataTable = ParkourDataAsset.Object;

	static ConstructorHelpers::FObjectFinder<USoundBase> ParkourCue(
		TEXT("/Game/Assets/Sounds/VR_ungrab_Cue.VR_ungrab_Cue"));
	if (ParkourCue.Succeeded())
		DoParkourSound = ParkourCue.Object;
}

void UCParkourComponent::BeginPlay()
{
	Super::BeginPlay();

	ResolveOwningCharacterCached();
	Moving =
		IsValid(OwningCharacter.Get()) ? OwningCharacter->FindComponentByClass<UCMovementComponent>() : nullptr;
	State =
		IsValid(OwningCharacter.Get()) ? OwningCharacter->FindComponentByClass<UCStateComponent>() : nullptr;

	ResolveArrowGroupResolved();
	RebuildArrowListFromArrowGroupChildren();
	RebuildHitResultSlotsMatchingArrowEnumOrder();
	LoadDatasFromParkourTable();

	if (false == IsValid(Moving.Get()) && IsValid(OwningCharacter.Get()))
		CLog::Log(FString(TEXT("UCParkourComponent: MovingComponent 를 캐시하지 못했다. 이동 연동 필요 시 검사")));

	if (false == IsValid(State.Get()) && IsValid(OwningCharacter.Get()))
		CLog::Log(FString(TEXT("UCParkourComponent: StateComponent 를 캐시하지 못했다. 상태 연동 필요 시 검사")));
}

void UCParkourComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckTrace_Center();

	if (IsValid(HitObstacle.Get()))
	{
		LineTrace(CEParkourArrowType::Floor);
		LineTrace(CEParkourArrowType::Land);
		LineTrace(CEParkourArrowType::Left);
		if (WallParkourArrowType >= CEParkourArrowType::Ceil &&
			WallParkourArrowType < CEParkourArrowType::Max &&
			WallParkourArrowType != CEParkourArrowType::Center)
		{
			LineTrace(WallParkourArrowType);
		}
	}
	else
	{
		CheckTrace_Land();
	}
}

void UCParkourComponent::ResolveOwningCharacterCached()
{
	OwningCharacter = Cast<ACharacter>(GetOwner());

	if (false == IsValid(OwningCharacter.Get()))
		CLog::Log(FString(TEXT("UCParkourComponent: Owner 가 유효한 ACharacter 로 캐스팅되지 않았다.")));
}

void UCParkourComponent::ResolveArrowGroupResolved()
{
	ArrowGroup = nullptr;

	if (false == IsValid(OwningCharacter.Get()))
		return;

	TArray<UActorComponent*> Components;
	OwningCharacter->GetComponents(Components);

	static const FName arrowsTag(TEXT("Arrows"));

	for (UActorComponent* Elem : Components)
	{
		if (false == IsValid(Elem))
			continue;

		USceneComponent* Scene = Cast<USceneComponent>(Elem);
		if (false == IsValid(Scene))
			continue;

		if (Elem->ComponentHasTag(arrowsTag))
		{
			ArrowGroup = Scene;
			return;
		}
	}

	for (UActorComponent* Elem : Components)
	{
		if (Elem != nullptr && Elem->GetFName() == FName(TEXT("ArrowGroup")))
		{
			ArrowGroup = Cast<USceneComponent>(Elem);
			if (IsValid(ArrowGroup.Get()))
				return;
		}
	}

	CLog::Log(FString::Printf(TEXT("UCParkourComponent('%s'): ArrowGroup 을 찾지 못했다."),
		IsValid(OwningCharacter.Get()) ? *OwningCharacter->GetName() : TEXT("(null character)")));
}

void UCParkourComponent::RebuildArrowListFromArrowGroupChildren()
{
	const int32 MaxSlots = static_cast<int32>(CEParkourArrowType::Max);
	Arrows.SetNum(MaxSlots);
	for (int32 IdxFill = 0; IdxFill < MaxSlots; IdxFill++)
		Arrows[IdxFill] = nullptr;

	if (false == IsValid(ArrowGroup.Get()))
		return;

	TArray<USceneComponent*> Children;
	ArrowGroup->GetChildrenComponents(/*bIncludeAllDescendants=*/false, Children);

	for (uint8 Raw = 0; Raw < static_cast<uint8>(CEParkourArrowType::Max); ++Raw)
	{
		const CEParkourArrowType SlotType = static_cast<CEParkourArrowType>(Raw);
		const FName TargetName = ResolveArrowActorSubobjectName(SlotType);

		UArrowComponent* MatchedArrow = nullptr;
		if (TargetName.IsNone())
			continue;

		const int32 ChildrenCount = Children.Num();
		for (int32 CIdx = 0; CIdx < ChildrenCount; CIdx++)
		{
			USceneComponent* Child = Children[CIdx];
			if (false == IsValid(Child))
				continue;

			UArrowComponent* ArrowCand = Cast<UArrowComponent>(Child);
			if (false == IsValid(ArrowCand))
				continue;

			const FName ChildName = ArrowCand->GetFName();
			if (ChildName == TargetName ||
				(SlotType == CEParkourArrowType::Land && ChildName == FName(TEXT("Land"))))
			{
				MatchedArrow = ArrowCand;
				break;
			}
		}

		const int32 EnumIndex = static_cast<int32>(SlotType);
		Arrows[EnumIndex] = MatchedArrow;
	}

	bool bMissingAnyEssentialArrow = false;
	static const TArray<CEParkourArrowType> RequiredArrows({
		CEParkourArrowType::Center,
		CEParkourArrowType::Land,
	});

	for (CEParkourArrowType Req : RequiredArrows)
	{
		const UArrowComponent* ReqArrow = ResolveArrowComponentForParkourArrowType(Req);
		if (ReqArrow == nullptr)
			bMissingAnyEssentialArrow = true;
	}

	if (bMissingAnyEssentialArrow != false)
		CLog::Log(FString::Printf(TEXT("UCParkourComponent: ArrowGroup '%s' 에서 필요한 UArrowComponent 슬롯을 모두 채우지 못했다."),
			IsValid(ArrowGroup.Get()) ? *ArrowGroup->GetName() : TEXT("(null ArrowGroup)")));
}

void UCParkourComponent::RebuildHitResultSlotsMatchingArrowEnumOrder()
{
	HitResults.Init(FHitResult(), static_cast<int32>(CEParkourArrowType::Max));
}

void UCParkourComponent::LoadDatasFromParkourTable()
{
	Datas.Reset();

	if (false == IsValid(DataTable.Get()))
		return;

	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		const FParkourData* FoundRow =
			DataTable->FindRow<FParkourData>(RowName, TEXT("Parkour_LoadDatas"));
		if (FoundRow != nullptr)
			Datas.Add(*FoundRow);
	}
}

UArrowComponent* UCParkourComponent::GetArrowComponent(
	const CEParkourArrowType InParkourArrowType) const
{
	if (false == (
		InParkourArrowType >= CEParkourArrowType::Ceil && InParkourArrowType < CEParkourArrowType::Max))
		return nullptr;

	const int32 Idx = static_cast<int32>(InParkourArrowType);
	if (false == Arrows.IsValidIndex(Idx))
		return nullptr;

	UArrowComponent* OutArrow = Arrows[Idx].Get();
	return IsValid(OutArrow) ? OutArrow : nullptr;
}

FHitResult UCParkourComponent::GetHitResultByArrowType(const CEParkourArrowType InParkourArrowType) const
{
	const int32 Idx = static_cast<int32>(InParkourArrowType);
	if (HitResults.IsValidIndex(Idx) == false)
		return FHitResult();

	return HitResults[Idx];
}

void UCParkourComponent::GetDatas(const CEParkourType InParkourType,
	TArray<FParkourData>& OutMatchingRows) const
{
	OutMatchingRows.Reset();
	const int32 RowCount = Datas.Num();

	for (int32 Ri = 0; Ri < RowCount; Ri++)
	{
		const FParkourData& Row = Datas[Ri];

		if (Row.Type != InParkourType)
			continue;

		OutMatchingRows.Add(Row);
	}
}

FParkourData UCParkourComponent::GetData(const CEParkourType InParkourType, const int32 InIndex) const
{
	TArray<FParkourData> Rows;
	GetDatas(InParkourType, Rows);

	if (Rows.IsValidIndex(InIndex))
		return Rows[InIndex];

	if (InIndex >= 0)
		CLog::Log(FString(TEXT("UCParkourComponent::GetData: InIndex 가 필터 결과 범위를 벗어났다. 빈 행 반환")));

	return FParkourData{};
}

void UCParkourComponent::PlayParkourMontage(const CEParkourType InParkourType, const int32 InIndex)
{
	if (false == IsValid(OwningCharacter.Get()))
	{
		CLog::Log(FString(TEXT("UCParkourComponent::PlayParkourMontage: 소유 Character 가 없어 조기 종료된다.")));
		return;
	}

	CurrentType = InParkourType;
	const FParkourData Row = GetData(InParkourType, InIndex);

	if (Row.bFixedCamera != false && IsValid(Moving.Get()))
		Moving->FixCamera();

	if (Row.Montage != nullptr && IsValid(Row.Montage.Get()))
		OwningCharacter->PlayAnimMontage(
			Row.Montage.Get(),
			Row.PlayRatio,
			Row.Section);

	if (IsValid(State.Get()))
		State->SetParkour();
}

bool UCParkourComponent::Check_DoParkour()
{
	CheckDoParkour_LastImpactPoint = FVector::ZeroVector;
	CheckDoParkour_LastImpactNormal = FVector::ZeroVector;
	CheckDoParkour_LastLookAtYawDegrees = 0.0;

	if (false == IsValid(OwningCharacter.Get()))
		return false;

	const FHitResult CenterHit =
		GetHitResultByArrowType(CEParkourArrowType::Center);
	const FHitResult WallHit =
		GetHitResultByArrowType(WallParkourArrowType);
	const FHitResult LandHit =
		GetHitResultByArrowType(CEParkourArrowType::Land);

	if (CenterHit.IsValidBlockingHit() == false || WallHit.IsValidBlockingHit() == false ||
		LandHit.IsValidBlockingHit() == false)
		return false;

	if (IsNearlyZeroSkipLogVector2D_XY(CenterHit.ImpactPoint) != false ||
		IsNearlyZeroSkipLogVector2D_XY(WallHit.ImpactPoint) != false)
		return false;

	CheckDoParkour_LastImpactPoint = CenterHit.ImpactPoint;
	CheckDoParkour_LastImpactNormal = CenterHit.ImpactNormal;

	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(
		OwningCharacter->GetActorLocation(),
		CenterHit.ImpactPoint);
	CheckDoParkour_LastLookAtYawDegrees = static_cast<double>(FRotator::NormalizeAxis(LookAtRotation.Yaw));

	const FVector ReferenceNormalTowardPlayer =
		-CenterHit.ImpactNormal.GetSafeNormal();
	if (ParkourYawWithinFrontDegrees(
		OwningCharacter->GetActorForwardVector(),
		ReferenceNormalTowardPlayer,
		AvailableFrontAngle) == false)
		return false;

	return true;
}

void UCParkourComponent::DoParkour()
{
	if (false == IsValid(OwningCharacter.Get()))
		return;

	if (CurrentType == CEParkourType::None)
		return;

	if (false == IsValid(State.Get()) || State->IsIdle() == false)
		return;

	if (Check_DoParkour() == false)
		return;

	if (IsValid(DoParkourSound.Get()))
		UGameplayStatics::PlaySoundAtLocation(this, DoParkourSound.Get(), OwningCharacter->GetActorLocation(),
			DoParkourSoundVolume);

	switch (CurrentType)
	{
	case CEParkourType::Climb:
		if (Check_ClimbMode() == false)
			return;
		DoParkour_ClimbMode();
		break;
	case CEParkourType::Slide:
		if (Check_SlideMode() == false)
			return;
		DoParkour_SlideMode();
		break;
	case CEParkourType::Vault:
	case CEParkourType::Mantle:
	case CEParkourType::Ledge:
	{
		int32 Idx = INDEX_NONE;
		if (Check_ObstacleMode(CurrentType, Idx) == false || Idx < 0)
			return;
		DoParkour_ObstacleMode(CurrentType, Idx);
		break;
	}
	default:
		if (bDoParkourLogWhenModeGraphMissingInCpp != false)
			CLog::Log(FString(TEXT("UCParkourComponent::DoParkour: CurrentType 브랜치가 C++ 에 없다. 레거시 BP 참고")));

		break;
	}
}

void UCParkourComponent::End_DoParkour()
{
	if (IsValid(Moving.Get()))
		Moving->UnFixCamera();

	const CEParkourType PrevType = CurrentType;
	CurrentType = CEParkourType::None;

	if (false == IsValid(OwningCharacter.Get()))
		return;

	UCharacterMovementComponent* Movement = OwningCharacter->GetCharacterMovement();

	switch (PrevType)
	{
	case CEParkourType::Climb:
		if (Movement != nullptr)
			Movement->SetMovementMode(MOVE_Walking);
		break;
	case CEParkourType::Slide:
	case CEParkourType::Vault:
	case CEParkourType::Mantle:
	case CEParkourType::Wallrun:
	case CEParkourType::Ledge:
	case CEParkourType::Hanging:
		if (AActor* BK = HitObstacle_Backup.Get())
			BK->SetActorEnableCollision(true);
		break;
	default:
		break;
	}
}

void UCParkourComponent::DoParkour_LandMode()
{
	if (false == IsValid(OwningCharacter.Get()))
		return;

	/* 블루프린트 "Do Parkour Land Mode": Current 가 Wallrun 또는 Vault 일 때 Idle 이고 착지 판정이면 Vault 랜드 몽타주. */
	const bool TypeOk =
		(CurrentType == CEParkourType::Vault) ||
		(CurrentType == CEParkourType::Wallrun);
	if (TypeOk == false)
		return;

	if (false == IsValid(State.Get()) || State->IsIdle() == false)
		return;

	if (Check_FallMode() == false)
		return;

	PlayParkourMontage(CEParkourType::Vault, 0);
}

bool UCParkourComponent::Check_ClimbMode() const
{
	const FHitResult CenterHit =
		GetHitResultByArrowType(CEParkourArrowType::Center);
	if (CenterHit.IsValidBlockingHit() == false)
		return false;

	const FParkourData Row = GetData(CEParkourType::Climb, 0);
	const double HitDist = HitObstacle_HitDistance;
	const double MinDist = static_cast<double>(Row.MinDistance);
	const double MaxDist = static_cast<double>(Row.MaxDistance);

	if (!(MinDist < HitDist))
		return false;
	if (!(MaxDist > HitDist))
		return false;

	if (FMath::IsNearlyEqual(
			static_cast<double>(Row.Extent),
			static_cast<double>(HitObstacle_Extent.Z),
			10.0) == false)
		return false;

	return true;
}

bool UCParkourComponent::Check_FallMode()
{
	/*
	 블루프린트 "Check_FallMode": Falling 이 true 로 들어오면 해당 프레임에 false 로 돌린 뒤 Land 화살표 트레이스 거리를
	 Vault 행(index 0) MinDistance - 200 과 비교한다.
	 Falling 이 false 로만 호출되는 경로에서는 정상이라도 항상 false 에 가까울 수 있어, 레벨 디자인 검증에 유의한다.
	*/
	if (bFalling != false)
		bFalling = false;

	const FHitResult LandHit =
		GetHitResultByArrowType(CEParkourArrowType::Land);
	const FParkourData VaultProbe = GetData(CEParkourType::Vault, 0);
	const double Threshold =
		static_cast<double>(VaultProbe.MinDistance) - 200.0;

	const double Distance = static_cast<double>(LandHit.Distance);
	return Distance > Threshold;
}

bool UCParkourComponent::Check_SlideMode()
{
	const FHitResult FloorHit =
		GetHitResultByArrowType(CEParkourArrowType::Floor);
	if (FloorHit.IsValidBlockingHit() == false)
		return false;

	const FParkourData SlideRow =
		GetData(CEParkourType::Slide, 0);

	const double HitDist = HitObstacle_HitDistance;
	if (!(static_cast<double>(SlideRow.MinDistance) < HitDist))
		return false;
	if (!(static_cast<double>(SlideRow.MaxDistance) > HitDist))
		return false;

	if (false == IsValid(OwningCharacter.Get()))
		return false;

	UArrowComponent* FloorArrow =
		GetArrowComponent(CEParkourArrowType::Floor);
	if (FloorArrow == nullptr || IsValid(FloorArrow) == false)
		return false;

	UWorld* World = GetWorld();
	if (World == nullptr)
		return false;

	const double FloorExtentD = static_cast<double>(SlideRow.Extent);

	const FVector FloorLoc = FloorArrow->GetComponentLocation();
	const FVector TraceStart(
		FloorLoc.X,
		FloorLoc.Y,
		FloorLoc.Z + 1.0);

	const FVector ActorForward =
		OwningCharacter.Get()->GetActorForwardVector();
	const FVector ProjXY(ActorForward.X, ActorForward.Y, 0.0);
	const FVector Dir = ProjXY.GetSafeNormal();
	if (Dir.IsNearlyZero())
		return false;

	const FVector TraceEnd =
		TraceStart +
		Dir * static_cast<double>(TraceDistance);

	const FVector HalfSize(
		0.0,
		FloorExtentD,
		FloorExtentD);

	const FRotator Orient =
		OwningCharacter.Get()->GetActorRotation();

	TArray<AActor*> Ignores{};
	GatherTraceIgnoreActors(Ignores);

	FHitResult BoxHit{};
	const FLinearColor TraceLinear(FloorArrow->ArrowColor);

	/* 블루프린트: Trace Type Query 1 — Floor 화살표 기준 디버그 색 유지 */
	const bool bBlockingBoxHit =
		UKismetSystemLibrary::BoxTraceSingle(
			this,
			TraceStart,
			TraceEnd,
			HalfSize,
			Orient,
			ETraceTypeQuery::TraceTypeQuery1,
			false,
			Ignores,
			DrawDebug,
			BoxHit,
			true,
			TraceLinear,
			FLinearColor::Green,
			5.f);

	return false == bBlockingBoxHit;
}

bool UCParkourComponent::Check_ObstacleMode(
	const CEParkourType InParkourType,
	int32& OutIndex) const
{
	OutIndex = INDEX_NONE;

	const FHitResult CenterHit =
		GetHitResultByArrowType(CEParkourArrowType::Center);

	/*
	 덤프상 조건은 "Center 비차단" 인데 이때에는 HitObstacle_* 가 채워지지 않아 루프 거리 검사와 모순된다.
	 실제 레거시가 그렇다면 아래 줄을 되돌린 뒤 별도 히트 데이터 경로가 있는지 검증해야 한다.
	*/
	if (CenterHit.IsValidBlockingHit() == false)
		return false;

	const double HitDist = HitObstacle_HitDistance;

	TArray<FParkourData> Rows{};
	GetDatas(InParkourType, Rows);

	const int32 RowNum = Rows.Num();
	for (int32 Ri = 0; Ri < RowNum; Ri++)
	{
		const FParkourData& Row = Rows[Ri];

		const bool LessMinThanHit =
			static_cast<double>(Row.MinDistance) < HitDist;
		const bool GreaterMaxThanHit =
			static_cast<double>(Row.MaxDistance) > HitDist;
		const bool ExtentAgainstYMatches =
			FMath::IsNearlyEqual(
				static_cast<double>(Row.Extent),
				static_cast<double>(HitObstacle_Extent.Y),
				10.0);

		if (LessMinThanHit && GreaterMaxThanHit && ExtentAgainstYMatches)
		{
			OutIndex = Ri;
			return true;
		}
	}

	return false;
}

void UCParkourComponent::DoParkour_ClimbMode()
{
	if (false == IsValid(OwningCharacter.Get()))
		return;

	ACharacter* Ch = OwningCharacter.Get();

	/* 블루프린트: Roll·Pitch 0, Yaw 만 FrontYaw (FRotator 는 Pitch, Yaw, Roll 순). */
	const FRotator OrientClimbRot(
		0.f,
		static_cast<float>(HitObstacle_FrontYaw),
		0.f);
	Ch->SetActorRotation(OrientClimbRot, ETeleportType::None);

	Ch->SetActorLocation(HitObstacle_HitPoint, false, nullptr, ETeleportType::None);

	PlayParkourMontage(CEParkourType::Climb, 0);

	UCharacterMovementComponent* CharacterMovementLocal =
		Ch->GetCharacterMovement();
	if (CharacterMovementLocal != nullptr)
		CharacterMovementLocal->SetMovementMode(MOVE_Flying);
}

void UCParkourComponent::DoParkour_SlideMode()
{
	if (false == IsValid(OwningCharacter.Get()))
		return;

	ACharacter* Ch = OwningCharacter.Get();

	const FRotator OrientSlideRot(
		0.f,
		static_cast<float>(HitObstacle_FrontYaw),
		0.f);
	Ch->SetActorRotation(OrientSlideRot, ETeleportType::None);

	PlayParkourMontage(CEParkourType::Slide, 0);

	HitObstacle_Backup = HitObstacle.Get();
	if (AActor* StoredObstacle =
		HitObstacle_Backup.Get())
		StoredObstacle->SetActorEnableCollision(false);
}

void UCParkourComponent::DoParkour_ObstacleMode(const CEParkourType InParkourType,
	const int32 InIndex)
{
	if (false == IsValid(OwningCharacter.Get()))
		return;

	ACharacter* Ch = OwningCharacter.Get();

	const FRotator OrientObstacleRot(
		0.f,
		static_cast<float>(HitObstacle_FrontYaw),
		0.f);
	Ch->SetActorRotation(OrientObstacleRot, ETeleportType::None);

	HitObstacle_Backup = HitObstacle.Get();
	if (AActor* StoredObstacle = HitObstacle_Backup.Get())
		StoredObstacle->SetActorEnableCollision(false);

	PlayParkourMontage(InParkourType, InIndex);
}

UArrowComponent* UCParkourComponent::ResolveArrowComponentForParkourArrowType(
	const CEParkourArrowType InType) const
{
	return GetArrowComponent(InType);
}

void UCParkourComponent::GatherTraceIgnoreActors(TArray<AActor*>& OutActors) const
{
	OutActors.Reset();
	if (IsValid(OwningCharacter.Get()))
		OutActors.Add(OwningCharacter.Get());
}

void UCParkourComponent::ClearParkourHitObstacle()
{
	HitObstacle_Backup = HitObstacle;
	HitObstacle = nullptr;
	HitObstacle_Extent = FVector::ZeroVector;
	HitObstacle_HitPoint = FVector::ZeroVector;
	HitObstacle_HitDistance = 0.0;
	HitObstacle_FrontYaw = 0.0;
}

void UCParkourComponent::CheckTrace_Center()
{
	if (false == IsValid(OwningCharacter.Get()))
		return;
	ClearParkourHitObstacle();
	LineTrace(CEParkourArrowType::Center);

	const FHitResult CenterHit =
		GetHitResultByArrowType(CEParkourArrowType::Center);

	if (CenterHit.IsValidBlockingHit() == false)
		return;

	const UArrowComponent* CenterArrowComp = GetArrowComponent(CEParkourArrowType::Center);
	if (CenterArrowComp == nullptr)
		return;

	AActor* HitActor = CenterHit.GetActor();

	if (false == IsValid(HitActor) || HitActor == OwningCharacter.Get())
		return;

	HitObstacle = HitActor;
	HitObstacle_HitPoint = CenterHit.ImpactPoint;

	const FVector TraceStart = CenterArrowComp->GetComponentLocation();
	HitObstacle_HitDistance = static_cast<double>(FVector::Dist(TraceStart, CenterHit.ImpactPoint));

	{
		const FVector PlaneFace = FVector::VectorPlaneProject(
			-CenterHit.ImpactNormal,
			FVector::UpVector);
		const FVector PlaneDir = PlaneFace.GetSafeNormal();
		if (PlaneDir.IsNearlyZero(KINDA_SMALL_NUMBER))
			HitObstacle_FrontYaw = 0.0;
		else
		{
			const float Yaw = UKismetMathLibrary::MakeRotFromX(PlaneDir).Yaw;
			HitObstacle_FrontYaw = static_cast<double>(FRotator::NormalizeAxis(Yaw));
		}
	}

	UStaticMeshComponent* MeshComp = HitActor->FindComponentByClass<UStaticMeshComponent>();
	HitObstacle_Extent = FVector::ZeroVector;
	if (MeshComp != nullptr && IsValid(MeshComp))
	{
		FVector LocalMin{};
		FVector LocalMax{};
		MeshComp->GetLocalBounds(LocalMin, LocalMax);
		const FVector Sz = (LocalMax - LocalMin).GetAbs();
		HitObstacle_Extent = Sz;
	}
}

void UCParkourComponent::LineTrace(const CEParkourArrowType InParkourArrowType)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
		return;

	if (false == IsValid(OwningCharacter.Get()))
		return;

	const int32 Idx = static_cast<int32>(InParkourArrowType);
	if (Idx < 0 || HitResults.Num() <= Idx ||
		false == (
			InParkourArrowType >= CEParkourArrowType::Ceil && InParkourArrowType < CEParkourArrowType::Max))
		return;

	UArrowComponent* ArrowObject = GetArrowComponent(InParkourArrowType);
	if (ArrowObject == nullptr)
		return;

	TArray<AActor*> Ignores;
	GatherTraceIgnoreActors(Ignores);

	const FVector TraceStart = ArrowObject->GetComponentLocation();
	const FVector TraceEnd =
		TraceStart + OwningCharacter->GetActorForwardVector() * static_cast<float>(TraceDistance);

	FHitResult HitOut{};
	const FLinearColor TraceColorLin = FLinearColor(ArrowObject->ArrowColor);

	UKismetSystemLibrary::LineTraceSingle(
		this,
		TraceStart,
		TraceEnd,
		ETraceTypeQuery::TraceTypeQuery3,
		false,
		Ignores,
		DrawDebug,
		HitOut,
		true,
		TraceColorLin,
		FLinearColor::Red,
		0.f);

	HitResults[Idx] = HitOut;
}

void UCParkourComponent::CheckTrace_Land()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
		return;

	if (false == IsValid(OwningCharacter.Get()))
		return;

	UCharacterMovementComponent* MovementComp = OwningCharacter->GetCharacterMovement();
	if (MovementComp == nullptr || MovementComp->IsFalling() == false)
		return;

	UArrowComponent* LandArrowObj = GetArrowComponent(CEParkourArrowType::Land);

	FVector TraceStart = FVector::ZeroVector;

	if (IsValid(LandArrowObj))
		TraceStart = LandArrowObj->GetComponentLocation();
	else
		TraceStart = OwningCharacter->GetActorLocation();

	TArray<AActor*> Ignores;
	GatherTraceIgnoreActors(Ignores);

	const FParkourData LandProbeRow =
		GetData(CEParkourType::Climb, 0);

	float ReachForward = LandProbeRow.Extent;
	if (ReachForward <= KINDA_SMALL_NUMBER)
		ReachForward = static_cast<float>(TraceDistance);

	const FVector DirForward = OwningCharacter->GetActorForwardVector();

	const FVector TraceEnd = TraceStart + DirForward * ReachForward;

	FHitResult HitOut{};

	const bool HadHitBlocking = UKismetSystemLibrary::LineTraceSingle(
		this,
		TraceStart,
		TraceEnd,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		Ignores,
		DrawDebug,
		HitOut,
		true);

	const int32 LandIndex = static_cast<int32>(CEParkourArrowType::Land);

	if (LandIndex >= 0 && LandIndex < HitResults.Num())
		HitResults[LandIndex] = HitOut;

	if (HadHitBlocking != false && HitOut.IsValidBlockingHit() != false)
		bFalling = false;
	else
		bFalling = true;
}
