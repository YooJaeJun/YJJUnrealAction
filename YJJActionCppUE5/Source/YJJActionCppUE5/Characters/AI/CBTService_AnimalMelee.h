#pragma once

#include "Characters/AI/CBTService_Character.h"
#include "CBTService_AnimalMelee.generated.h"

/**
 * 레거시 `/Game/Character/Animals/BTService_Animal_Melee` — `BTService_Character` 상속 블프 `Tick AI` 포팅.
 * 근거리·Idle/Rise·타겟 없음 시 트레이스 실패에 따른 Behavior 블랙보드 기록 순서를 맞춘다.
 *
 * 블프 덤프의 `NewEnumerator*` 정수 표기와 Content 의 `EBehaviorType` 순서가 틀릴 수 있으므로
 * `BehaviorByte_*` 는 BT 에디터에서 UDE 순서 확인 후 교정한다(예: 근거리 17·멀 Idle/Rise 분기 11·트레이스 실패 18).
 */
UCLASS(meta = (DisplayName = "BTService Animal Melee"))
class YJJACTIONCPPUE5_API UCBTService_AnimalMelee : public UCBTService_Character
{
	GENERATED_BODY()

public:
	UCBTService_AnimalMelee();

protected:
	/** 레거시 BP `ActionDistance`(cm), 기본 300. `GetDistanceTo` 결과와 같은 단위비교. */
	UPROPERTY(EditAnywhere, Category = "Melee|Distance", meta = (ClampMin = "1"))
	double ActionDistance = 300.0;

	/** `Distance <= ActionDistance` 일 때 Behavior 키에 기록하는 UDE 바이트(블프 NewEnumerator17 근사). */
	UPROPERTY(EditAnywhere, Category = "Melee|BB Bytes", meta = (ClampMin = "0"))
	uint8 BehaviorByte_InMeleeRange = 17;

	/** 멀리 있고 `CurState` 가 Idle 또는 Rise 일 때 Behavior 키 바이트(블프 NewEnumerator11 근사). */
	UPROPERTY(EditAnywhere, Category = "Melee|BB Bytes", meta = (ClampMin = "0"))
	uint8 BehaviorByte_OutOfRangeIdleOrRise = 11;

	/** 타깃 무효 + `TryTraceAcquireTarget`(레거시 TraceTargetWhenNullTarget) 실패 시 바이트(블프 NewEnumerator18 근사). */
	UPROPERTY(EditAnywhere, Category = "Melee|BB Bytes", meta = (ClampMin = "0"))
	uint8 BehaviorByte_TraceMissed = 18;

	virtual void TickCharacterAi(UBehaviorTreeComponent& OwnerComp) override;
};
