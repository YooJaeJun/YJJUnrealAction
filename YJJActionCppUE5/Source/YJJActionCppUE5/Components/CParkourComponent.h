#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "CollisionQueryParams.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Commons/CEnums.h"
#include "Parkour/FParkourData.h"
#include "CParkourComponent.generated.h"

class USceneComponent;
class UArrowComponent;
class UCStateComponent;
class UCMovementComponent;

/**
 * 레거시 `/Game/Components/ParkourComponent` 블루프린트를 C++ 로 옮긴 컴포넌트.
 * 블루프린트 `EParkourArrowType` 과 동일 순서라고 가정하고 `Arrows[enum 값]` 과 `HitResults` 를 맞춘다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YJJACTIONCPPUE5_API UCParkourComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCParkourComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour",
		meta = (BlueprintPrivate = "true", DisplayName = "Arrow Group"))
	TObjectPtr<USceneComponent> ArrowGroup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour", meta = (BlueprintPrivate = "true"))
	TObjectPtr<UCMovementComponent> Moving;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour|DataTables", meta = (MultiLine = "true"))
	TObjectPtr<UDataTable> DataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour|DataTables", meta = (MultiLine = "true"))
	TArray<FParkourData> Datas;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Obstacles")
	TObjectPtr<AActor> HitObstacle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Obstacles")
	FVector HitObstacle_Extent = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Obstacles")
	FVector HitObstacle_HitPoint = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Obstacles")
	double HitObstacle_HitDistance = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Obstacles")
	double HitObstacle_FrontYaw = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Obstacles")
	TObjectPtr<AActor> HitObstacle_Backup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour|Trace", meta = (MultiLine = "true"))
	double TraceDistance = 600.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour|Trace", meta = (MultiLine = "true"))
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebug = EDrawDebugTrace::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour|Trace", meta = (MultiLine = "true"))
	double AvailableFrontAngle = 60.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour", meta = (BlueprintPrivate = "true", DisplayName = "Character"))
	TObjectPtr<class ACharacter> OwningCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour", meta = (BlueprintPrivate = "true"))
	TArray<TObjectPtr<UArrowComponent>> Arrows;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour", meta = (BlueprintPrivate = "true"))
	TArray<FHitResult> HitResults;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour", meta = (BlueprintPrivate = "true"))
	TObjectPtr<UCStateComponent> State;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour", meta = (BlueprintPrivate = "true", DisplayName = "Current Type"))
	CEParkourType CurrentType = CEParkourType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour", meta = (MultiLine = "true"))
	bool bFalling = false;

	/** 레거시 BP 에 `Wall`(벽 검사 화살표) 이름이 따로 없을 때 우측 화살표를 매핑한다. 에셋이 다르면 교체해야 한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour|Trace", meta = (MultiLine = "true"))
	CEParkourArrowType WallParkourArrowType = CEParkourArrowType::Right;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour|Audio", meta = (MultiLine = "true"))
	TObjectPtr<class USoundBase> DoParkourSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour|Audio", meta = (MultiLine = "true"))
	float DoParkourSoundVolume = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour|Checks", meta = (MultiLine = "true"))
	bool bDoParkourLogWhenModeGraphMissingInCpp = false;

	/** `Check_DoParkour()` 가 마지막으로 채운 Center 기준 디버깅 정보(블루프린트 변수 대응). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Checks")
	FVector CheckDoParkour_LastImpactPoint = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Checks")
	FVector CheckDoParkour_LastImpactNormal = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Checks")
	double CheckDoParkour_LastLookAtYawDegrees = 0.0;

	UFUNCTION(BlueprintPure, Category = "Parkour|Helpers")
	UArrowComponent* GetArrowComponent(CEParkourArrowType InParkourArrowType) const;

	UFUNCTION(BlueprintPure, Category = "Parkour|Helpers")
	FHitResult GetHitResultByArrowType(CEParkourArrowType InParkourArrowType) const;

	UFUNCTION(BlueprintPure, Category = "Parkour|Helpers")
	void GetDatas(CEParkourType InParkourType, TArray<FParkourData>& OutMatchingRows) const;

	UFUNCTION(BlueprintPure, Category = "Parkour|Helpers")
	FParkourData GetData(CEParkourType InParkourType, int32 InIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Parkour|Anim")
	void PlayParkourMontage(CEParkourType InParkourType, int32 InIndex);

	UFUNCTION(BlueprintPure, Category = "Parkour|Checks")
	bool Check_DoParkour();

	UFUNCTION(BlueprintPure, Category = "Parkour|Checks")
	bool Check_ClimbMode() const;

	/** 낙하 판정용. `bFalling` 이 true 인 프레임에 호출되면 BP 와 같이 false 로 소비한 뒤 거리 식을 계산한다. */
	UFUNCTION(BlueprintCallable, Category = "Parkour|Checks")
	bool Check_FallMode();

	UFUNCTION(BlueprintCallable, Category = "Parkour|Checks")
	bool Check_SlideMode();

	UFUNCTION(BlueprintPure, Category = "Parkour|Checks")
	bool Check_ObstacleMode(CEParkourType InParkourType, int32& OutIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Parkour|Gameplay")
	void DoParkour_LandMode();

	UFUNCTION(BlueprintCallable, Category = "Parkour|Gameplay")
	void DoParkour();

	UFUNCTION(BlueprintCallable, Category = "Parkour|Gameplay")
	void End_DoParkour();

	UFUNCTION(BlueprintCallable, Category = "Parkour|Trace")
	void CheckTrace_Center();

	UFUNCTION(BlueprintCallable, Category = "Parkour|Trace")
	void LineTrace(CEParkourArrowType InParkourArrowType);

	UFUNCTION(BlueprintCallable, Category = "Parkour|Trace")
	void CheckTrace_Land();

private:
	void ResolveOwningCharacterCached();
	void ResolveArrowGroupResolved();
	void RebuildArrowListFromArrowGroupChildren();
	void RebuildHitResultSlotsMatchingArrowEnumOrder();
	void LoadDatasFromParkourTable();

	UArrowComponent* ResolveArrowComponentForParkourArrowType(CEParkourArrowType InType) const;
	void ClearParkourHitObstacle();
	void GatherTraceIgnoreActors(TArray<AActor*>& OutActors) const;

	void DoParkour_ClimbMode();
	void DoParkour_SlideMode();

	void DoParkour_ObstacleMode(CEParkourType InParkourType, int32 InIndex);
};
