#pragma once

#include "Characters/AI/CAIController.h"
#include "CAIController_Animal.generated.h"

class ACAnimal;
/**
 * 레거시 /Game/Character/Animals/BP_AIController_Animal — BP_AIController 상속 분기 포팅.
 * Possess 시 Animal_AI 캐시 후 부모 RunAI 로 폰의 BehaviorTree 에셋을 실행하고, 레거시와 동일하게 Sight 만 사용한다.
 */
UCLASS()
class YJJACTIONCPPUE5_API ACAIController_Animal : public ACAIController
{
	GENERATED_BODY()

public:
	ACAIController_Animal(const FObjectInitializer& ObjectInitializer);

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

protected:
	virtual bool ShouldUseHearingPerceptionSense() const override;
	virtual void ConfigureSightSenseAffiliation(UAISenseConfig_Sight& SightConfigRef) override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "세팅", meta = (AllowPrivateAccess = "true", DisplayName = "Animal AI"))
	TObjectPtr<ACAnimal> Animal_AI;

	/** 레거시 RunBehavior 변수(기본 false). 현재 Possess 플로우는 레거시 BP 와 동일하게 항상 RunAI 를 호출한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "세팅", meta = (MultiLine = "true", DisplayName = "Run Behavior"))
	bool bRunBehavior = false;

	/** 레거시 Behavior Tree Save 에셋(보통 미사용; 폰 BehaviorTree 또는 컨트롤러 BT 가 우선). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "세팅", meta = (MultiLine = "true", DisplayName = "Behavior Tree Save"))
	TObjectPtr<UBehaviorTree> BehaviorTreeSave;
};
