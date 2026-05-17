#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Commons/CEnums.h"
#include "CBTService_Character.generated.h"

class ACCommonCharacter;
class AAIController;

/**
 * 레거시 /Game/Character/BTService_Character BTService 블루프린트 — Tick AI / SetBehaviorType / SetTarget / Trace 분기 로직 포팅.
 * CEStateType·CEBehaviorType·블랙보드 키·NextMovingPoint·Attacker(피격) 흐름을 레거시와 맞춘다.
 */
UCLASS(meta = (DisplayName = "BTService Character"))
class YJJACTIONCPPUE5_API UCBTService_Character : public UBTService
{
	GENERATED_BODY()

public:
	UCBTService_Character();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "BB", meta = (DisplayName = "Behavior Key"))
	FBlackboardKeySelector BehaviorKey;

	UPROPERTY(EditAnywhere, Category = "BB", meta = (DisplayName = "Target Key"))
	FBlackboardKeySelector TargetKey;

	/** 레거시 "Is Set Behavior" — 블루프린트는 BehaviorKey 에 EBehaviorType(UE enum 바이트)을 썼다.
	 * 꺼 두면 상태 분기별 SetBlackboardBehavior 는 건너뛴다(Target 거울/트레이스/SetAITarget 은 계속 처리). */
	UPROPERTY(EditAnywhere, Category = "세팅", meta = (DisplayName = "Is Set Behavior"))
	bool bIsSetBehavior = true;

	/** 레거시 TraceTargetWhenNullTarget 호출 의도: 블랙보드 타겟이 비었을 때만 구체 스윕 후 찾으면 타겟 키에 기록한다. */
	UPROPERTY(EditAnywhere, Category = "세팅", meta = (DisplayName = "Trace When Missing Target"))
	bool bTraceWhenMissingBlackboardTarget = true;

	UPROPERTY(EditAnywhere, Category = "세팅", meta = (DisplayName = "Trace Radius"))
	float SphereTraceRadius = 1000.0f;

	// 레거시 BP 변수(표시 이름) 거울
	UPROPERTY(
		BlueprintReadOnly,
		Category = "세팅",
		meta = (AllowPrivateAccess = "true", DisplayName = "Target"))
	TObjectPtr<ACCommonCharacter> BlackboardTargetMirror;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "세팅",
		meta = (AllowPrivateAccess = "true", DisplayName = "Controller"))
	TObjectPtr<AAIController> OwningControllerRef;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "세팅",
		meta = (AllowPrivateAccess = "true", DisplayName = "Cur State"))
	CEStateType CurState = CEStateType::Idle;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Settings",
		meta = (AllowPrivateAccess = "true", DisplayName = "Character"))
	TObjectPtr<ACCommonCharacter> ControlledCharacterRef;

	virtual void TickCharacterAi(UBehaviorTreeComponent& OwnerComp);

	void ResolveOwnerAndControlledCharacter(UBehaviorTreeComponent& OwnerComp);
	bool TryRefreshCurStateFromStateComponent();
	void ApplyBlackboardTargetMirror(UBehaviorTreeComponent& OwnerComp);

	bool TryTraceAcquireTarget(UBehaviorTreeComponent& OwnerComp);

	/** 레거시 UDE(EBehaviorType) 순서와 `CEBehaviorType` 이 다를 때 BT Behavior 키에 기록해야 할 순수 바이트.
	 * `bIsSetBehavior`/Behavior 키 유효 검사 규칙은 `PushBehaviorBlackboard` 와 동일하다. */
	void PushBehaviorBlackboardRawByte(UBehaviorTreeComponent& OwnerComp, uint8 InBehaviorByte);

	void PushBehaviorBlackboard(UBehaviorTreeComponent& OwnerComp, CEBehaviorType InBehaviorType);
	/** 레거시 SetTarget: 같은 그룹이면 거짓을 반환(이동 타겟 갱신 없음). */
	bool SetAITarget(UBehaviorTreeComponent& OwnerComp, ACCommonCharacter* InTarget);

	ACCommonCharacter* ResolveBlackboardTargetCharacter(UBehaviorTreeComponent& OwnerComp) const;
	void TryWriteBlackboardTargetObject(UBehaviorTreeComponent& OwnerComp, ACCommonCharacter* InTarget);

private:
	void TickStateBranches(UBehaviorTreeComponent& OwnerComp);

	void RunIdleEquipActTargetFlow(UBehaviorTreeComponent& OwnerComp);
	void RunCombatHittedFlow(UBehaviorTreeComponent& OwnerComp);
	void RunRiseFlow(UBehaviorTreeComponent& OwnerComp);
};
