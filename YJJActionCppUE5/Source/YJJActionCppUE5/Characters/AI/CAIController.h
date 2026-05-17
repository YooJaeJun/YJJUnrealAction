#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "CAIController.generated.h"

class ACEnemy;
class UBehaviorTree;
class UBlackboardData;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;

UCLASS(Abstract)
class YJJACTIONCPPUE5_API ACAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACAIController(const FObjectInitializer& ObjectInitializer);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	void RunAI(APawn* InPossessedPawn);
	void StopAI() const;

	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

public:
	static const FName SelfActor;
	static const FName Target;
	static const FName Behavior;
	static const FName TargetLocation;

protected:
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "AI",
		meta = (AllowPrivateAccess = "true", DisplayName = "Enemy AI"))
	TObjectPtr<ACEnemy> EnemyAI;

	/** 레거시 BP_AIController 는 청각 포함, BP_AIController_Animal 은 Sight 만 사용한다. */
	virtual bool ShouldUseHearingPerceptionSense() const;
	virtual void ConfigureSightSenseAffiliation(UAISenseConfig_Sight& SightConfigRef);

private:

	UPROPERTY()
	TObjectPtr<UBehaviorTree> BTAsset;

	UPROPERTY()
	TObjectPtr<UBlackboardData> BBAsset;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(EditAnywhere)
	float SightRadius = 1500.0f;

	UPROPERTY(EditAnywhere)
	float LoseSightRadius = 1800.0f;

	UPROPERTY(EditAnywhere)
	float Angle = 120.0f;

	UPROPERTY(EditAnywhere)
	float MaxAge = 5.0f;

	void ProcessPerceptionAndUpdateBlackboardTarget();

	/** CDO 에서 ConstructorHelpers 로 BB/BT 를 박아 두면 BT·BP 순환으로 Async Flush 교착 — RunAI 시점에 디스크에서 보충한다. */
	void AiControllerEnsureDefaultBlackboardAndBehaviorTreeLoadedIfUnset();
};
