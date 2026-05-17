#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Commons/CEnums.h"
#include "CBTService_DragonBoss.generated.h"

class ACDragon;
class ACCommonCharacter;
class AAIController;

/**
 * 레거시 BTService_DragonBoss 블루프린트(TickActionAI·SetTarget·회전 보간·거리 밴드·블프 Enum 블랙보드 기록)의
 * 참조 깨짐(Dragon/Weapon/BF_Helpers) 우회용 네이티브 서비스.
 * 블랙보드 DistanceType·Behavior 필드 타입 바이트는 Content 의 UserDefinedEnum 과 동일해야 하며,
 * 디폴트 숫자는 블프 덤프의 NewEnumerator* 표기 근사 — 컴파일 후 BT 에셋에 맞춰 교정할 것.
 */
UCLASS(meta = (DisplayName = "BT Service (DragonBoss)"))
class YJJACTIONCPPUE5_API UCBTService_DragonBoss : public UBTService
{
	GENERATED_BODY()

public:
	UCBTService_DragonBoss();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "BB", meta = (DisplayName = "Behavior Key"))
	FBlackboardKeySelector BehaviorKey;

	UPROPERTY(EditAnywhere, Category = "BB", meta = (DisplayName = "Target Key"))
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, Category = "BB", meta = (DisplayName = "Distance Type Key"))
	FBlackboardKeySelector DistanceTypeKey;

	/** 레거시 Sphere Trace Radius (데미지 블프 100000). */
	UPROPERTY(EditAnywhere, Category = "Trace", meta = (ClampMin = "1"))
	float SphereTraceRadius = 100000.0f;

	/** 회전 차이 무시 허치(블프 NearlyEqual_FloatFloat). */
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0"))
	float RotationYawEpsilonDegrees = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0.01"))
	float RotationInterpSpeed = 1.5f;

	/** 레거시 Greater_DoubleDouble 기준값 (제곱거리.cm² 순서 동일 유지됨). */
	UPROPERTY(EditAnywhere, Category = "DistanceBands")
	double DistanceSquaredThreshold_Approach = 10000000.0;

	UPROPERTY(EditAnywhere, Category = "DistanceBands")
	double DistanceSquaredThreshold_Skill = 1000000.0;

	UPROPERTY(EditAnywhere, Category = "DistanceBands")
	double DistanceSquaredThreshold_Action = 600000.0;

	/** UDE 레거시 정수(`CEDistanceType::Approach` 기본 디폴트). */
	UPROPERTY(EditAnywhere, Category = "DragonBoss|BB Bytes", meta = (ClampMin = "0"))
	uint8 DistanceTypeBB_Approach = static_cast<uint8>(CEDistanceType::Approach);

	/** UDE 레거시 정수(`CEDistanceType::Skill`). */
	UPROPERTY(EditAnywhere, Category = "DragonBoss|BB Bytes", meta = (ClampMin = "0"))
	uint8 DistanceTypeBB_Skill = static_cast<uint8>(CEDistanceType::Skill);

	/** UDE 레거시 정수(`CEDistanceType::Action`). */
	UPROPERTY(EditAnywhere, Category = "DragonBoss|BB Bytes", meta = (ClampMin = "0"))
	uint8 DistanceTypeBB_Action = static_cast<uint8>(CEDistanceType::Action);

	/** UDE 레거시 정수(`CEDistanceType::Avoid`). */
	UPROPERTY(EditAnywhere, Category = "DragonBoss|BB Bytes", meta = (ClampMin = "0"))
	uint8 DistanceTypeBB_Avoid = static_cast<uint8>(CEDistanceType::Avoid);

	/** UDE 레거시 정수(`CEDragonBossBehaviorType::Approach`). */
	UPROPERTY(EditAnywhere, Category = "DragonBoss|BB Bytes", meta = (ClampMin = "0"))
	uint8 BossBehavior_ApproachBandByte = static_cast<uint8>(CEDragonBossBehaviorType::Approach);

	/** UDE 레거시 정수(`CEDragonBossBehaviorType::Action`). */
	UPROPERTY(EditAnywhere, Category = "DragonBoss|BB Bytes", meta = (ClampMin = "0"))
	uint8 BossBehavior_ActionBandByte = static_cast<uint8>(CEDragonBossBehaviorType::Action);

	/** UDE 레거시 정수(`CEDragonBossBehaviorType::Skill` — 블프는 Skill/Avoid 구간 등에 동일 값 사용). */
	UPROPERTY(EditAnywhere, Category = "DragonBoss|BB Bytes", meta = (ClampMin = "0"))
	uint8 BossBehavior_SkillAvoidBandByte = static_cast<uint8>(CEDragonBossBehaviorType::Skill);

	/** 상태 텍스트(UTextRenderComponent)를 동기화하면 BT 틱 비용 증가 — 필요 시 비활성화. */
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bUpdateStateTextRender = true;

private:
	void TickDragonBossAi(UBehaviorTreeComponent& OwnerComp, float DeltaSeconds);

	bool ResolveControlledDragon(UBehaviorTreeComponent& OwnerComp, ACDragon*& OutDragon);

	bool ResolveCurStateFromPawn(ACDragon* Dragon);

	void RefreshBlackboardTargetMirror(UBehaviorTreeComponent& OwnerComp);

	bool ShouldTickTargetAndFacingForCurrentState() const;

	void TickTargetTraceOrMaintain(UBehaviorTreeComponent& OwnerComp);

	void RotateDragonTowardBbTarget(ACDragon* Dragon, float DeltaSeconds);

	void EvaluateDistanceBandsAndBehavior(ACDragon* Dragon, UBehaviorTreeComponent& OwnerComp);

	void TraceAndResolveCombatTarget(ACDragon* Dragon, UBehaviorTreeComponent& OwnerComp);

	void AppendIgnoreActors(ACDragon* Dragon, TArray<AActor*>& InOutActors) const;

	void WriteBlackboardEnumIfSet(UBehaviorTreeComponent& OwnerComp, const FBlackboardKeySelector& Key,
		const uint8 Value);

	void PushBossBehavior(ACDragon* Dragon, UBehaviorTreeComponent& OwnerComp, const uint8 BehaviorByte);

	void TryWriteStateDebugText(ACCommonCharacter* TextSourceCharacter, uint8 BossBehaviorByte);

	TObjectPtr<AAIController> OwningAi;

	TObjectPtr<ACDragon> CachedDragon;

	TObjectPtr<ACCommonCharacter> BlackboardTargetMirror;

	/** 레거시 FindAndAdd — 배열 내 중복 불가 · 새로 들어간 경우만 SetTarget 과 연동. */
	TArray<ACCommonCharacter*> HittedUniqueAccumulator;

	CEStateType CurStateTyped = CEStateType::Idle;
};
