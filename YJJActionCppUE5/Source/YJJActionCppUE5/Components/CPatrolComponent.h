#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPatrolComponent.generated.h"

class AActor;
class USplineComponent;

/**
 * 순찰 타깃(스플라인)과 현재 waypoint 인덱스를 보관한다.
 * 기존 Blueprint PatrolComponent 의 IsValidPath / GetMoveTo / UpdateNextIndex 그래프와 동일하게 동작하도록 포팅.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YJJACTIONCPPUE5_API UCPatrolComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPatrolComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** 패스 오브젝트(BP_PatrolPass 등)가 존재하는지 — BP IsValid(Path) 동일 */
	UFUNCTION(BlueprintPure, Category = "Patrol")
	bool IsValidPath() const;

	/**
	 * 유효한 패스가 있으면 현재 Index 의 월드 위치를 반환한다.
	 * 무효면 OutResult=false, 위치 영벡터(레거시 BP 와 동일).
	 */
	UFUNCTION(BlueprintCallable, Category = "Patrol")
	void GetMoveTo(UPARAM(ref) bool& OutResult, UPARAM(ref) FVector& OutLocation);

	/** 다음 스플라인 포인트로 인덱스·Reverse 상태를 진행한다(레거시 UpdateNextIndex). */
	UFUNCTION(BlueprintCallable, Category = "Patrol")
	void UpdateNextIndex();

public:
	/** 목표 지점 허용 반경(UI/ BT 공용 디자인 값 — 그래프에선 미사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "세팅", meta = (ClampMin = "0.0"))
	float AcceptanceRadius = 20.f;

	/** 순찰 경로 액터(스플라인 보유 예: BP_PatrolPass_C) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TObjectPtr<AActor> Path = nullptr;

	/** 현재 스플라인 포인트 인덱스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 Index = 0;

	/** true 일 때 역방향 이동(인덱스 감소) 모드 — BP Reverse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool Reverse = false;

private:
	USplineComponent* ResolveSpline(AActor* PathActor) const;
	static bool TryReadBlueprintBoolProperty(UObject* Object, const FName PropertyName, bool& OutValue);
};
