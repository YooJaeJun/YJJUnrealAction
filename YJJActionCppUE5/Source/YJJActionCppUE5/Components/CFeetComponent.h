#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Characters/CCharacterFeetData.h"
#include "CFeetComponent.generated.h"

class ACharacter;
class USkeletalMeshComponent;

// ABP_Character 가 BP FeetComponent.Data 를 읽던 경로의 네이티브 대응 — 풋 IK 스냅샷을 애님에서 복사한다.
UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class YJJACTIONCPPUE5_API UCFeetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCFeetComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 레거시 BP 의 FFeetData 구조체(사용자 정의 구조체)와 동명.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|세팅", meta = (DisplayName = "Data"))
	FFeetData Data;

	// --- Trace /Game/Components/FeetComponent BPVar Category "Trace" ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Trace", meta = (DisplayName = "Trace Distance"))
	double TraceDistance = 50.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Trace", meta = (DisplayName = "Draw Debug"))
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebug = EDrawDebugTrace::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Trace", meta = (DisplayName = "Offset Distance"))
	double OffsetDistance = 5.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Trace", meta = (DisplayName = "Interp Speed"))
	double InterpSpeed = 50.0;

	// --- 세팅: 소켓·메시 (BP 카테고리 "세팅") ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|세팅", meta = (DisplayName = "Target Mesh"))
	TObjectPtr<USkeletalMeshComponent> TargetMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|세팅", meta = (DisplayName = "Point 1"))
	FName Point_1 = FName(TEXT("Foot_L"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|세팅", meta = (DisplayName = "Point 2"))
	FName Point_2 = FName(TEXT("Foot_R"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|세팅", meta = (DisplayName = "Point 3"))
	FName Point_3 = FName(TEXT("Hand_L"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|세팅", meta = (DisplayName = "Point 4"))
	FName Point_4 = FName(TEXT("Hand_R"));

	// BP 에서 BlueprintPrivate 였던 Character — 비우면 BeginPlay 에서 오너로 채운다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|세팅", meta = (DisplayName = "Character"))
	TObjectPtr<ACharacter> Character = nullptr;

	// 레거시 BP FeetComponent_C 의 IK 스칼라 (카테고리 Default) — ACAnimalAnimDynamicInfo::Tick_Info 가 스냅샷만 복사한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Default")
	double LeftFoot = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Default")
	double RightFoot = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Default")
	double LeftHand = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Default")
	double RightHand = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Default")
	double Pelvis = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Default", meta = (DisplayName = "Ground Slope"))
	FRotator GroundSlope = FRotator::ZeroRotator;

	// BP 와 동일 이름의 스칼라 거리(구조체 Data 의 FVector 과 별개 — 트레이스/블렌드용으로 병행 존재).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Distance", meta = (DisplayName = "Left Foot Distance"))
	double LeftFootDistance = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Distance", meta = (DisplayName = "Right Foot Distance"))
	double RightFootDistance = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Distance", meta = (DisplayName = "Left Hand Distance"))
	double LeftHandDistance = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Distance", meta = (DisplayName = "Right Hand Distance"))
	double RightHandDistance = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Distance", meta = (DisplayName = "Pelvis Distance"))
	double PelvisDistance = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Distance Save", meta = (DisplayName = "Left Foot Distance Save"))
	double LeftFootDistance_Save = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Distance Save", meta = (DisplayName = "Right Foot Distance Save"))
	double RightFootDistance_Save = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Distance Save", meta = (DisplayName = "Left Hand Distance Save"))
	double LeftHandDistance_Save = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Distance Save", meta = (DisplayName = "Right Hand Distance Save"))
	double RightHandDistance_Save = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Distance Save", meta = (DisplayName = "Pelvis Distance Save"))
	double PelvisDistance_Save = 0.0;

	// 컴포넌트 레벨 회전( Data 구조체 내부 회전 과 병행 가능).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Rotation", meta = (DisplayName = "Left Foot Rotation"))
	FRotator LeftFootRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Rotation", meta = (DisplayName = "Right Foot Rotation"))
	FRotator RightFootRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Rotation", meta = (DisplayName = "Left Hand Rotation"))
	FRotator LeftHandRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Rotation", meta = (DisplayName = "Right Hand Rotation"))
	FRotator RightHandRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet|Rotation", meta = (DisplayName = "Pelvis Rotation"))
	FRotator PelvisRotation = FRotator::ZeroRotator;

	// --- BP FeetComponent: Trace / Trace4Leg / IsPlayer ---
	UFUNCTION(
		BlueprintCallable,
		Category = "Feet|Trace",
		meta = (DisplayName = "Trace"))
	void TraceGroundFromSocketBP(FName InSocketName, USkeletalMeshComponent* InSkeletalMesh, double& OutDistance, FRotator& OutRotation, FVector& OutImpactPoint);

	UFUNCTION(BlueprintCallable, Category = "Feet|Trace", meta = (DisplayName = "Trace 4Leg"))
	bool TraceFourLegGroundBP(FVector InWorldTraceStart, FVector& OutHitLocation, FVector& OutHitNormal);

	// 레거시 BP IsPlayer 노드 미연결(항상 false)과 동등 — 새 로직 필요 시 교체한다.
	UFUNCTION(BlueprintPure, Category = "Feet", meta = (DisplayName = "Is Player"))
	bool LegacyGraph_IsPlayer_DefaultFalse() const { return false; }

private:
	// BP Apply_Inverse_Kinematics / Trace4Leg — 소켓 수직 트레이스·보간으로 Data·스칼라·경사 갱신.
	void ApplyInverseKinematics(double InDeltaSeconds);

	USkeletalMeshComponent* ResolveTargetMesh() const;

	/** FeetComponent.BP Trace 브랜치와 동등 (BreakHit 의 Normal 로 경사 회전 계산 등). */
	bool TraceLimbGroundMatchBPTrace(
		ACharacter* TraceCharacter,
		USkeletalMeshComponent* Mesh,
		FName SocketName,
		float CapsuleScaledHalfHeight,
		FHitResult& OutHit,
		double& OutRawDistanceSigned,
		FRotator& OutGroundRotation_FromHitNormal_NormalComponent) const;
};
