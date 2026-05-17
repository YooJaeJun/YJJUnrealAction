#pragma once

#include "CoreMinimal.h"
#include "Characters/CCommonCharacter.h"
#include "Weapons/CWeaponStructures.h"
#include "CEnemy.generated.h"

class AController;
class UBehaviorTree;
class UCWeaponComponent;

// 적 원형 — 과거 BP/C++ 에서 분리돼 있던 적·적 AI 초기 규칙을 ACEnemy 단일 클래스로 통합함.
UCLASS(Abstract)
class YJJACTIONCPPUE5_API ACEnemy : public ACCommonCharacter
{
	GENERATED_BODY()

public:
	ACEnemy();

	/** 레거시 BP_Enemy_AI::BehaviorTree — 비우면 ACAIController 가 CBT_Melee 등 기본 에셋을 사용한다. */
	UBehaviorTree* GetEnemyBehaviorTreeForController() const;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCWeaponComponent> WeaponComp;

	/** 레거시 BP 변수명 BehaviorTree 과 동일 — AI BP 가 Get/Set 모두 쓸 수 있게 쓰기 허용(Set 은 설계상 담당자만). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (MultiLine = "true"))
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	TArray<float> Speeds{ 200, 400, 600 };

private:
	UFUNCTION()
	void OnStateTypeChanged(CEStateType InPrevType, CEStateType InNewType);

	UFUNCTION()
	void OnHitStateTypeChanged(const CEHitType InPrevType, const CEHitType InNewType);

private:
	virtual void Hit() override;
	virtual void End_Hit() override;
	virtual void End_Rise() override;

private:
	CEHitType CurHitType = CEHitType::Common;
};
