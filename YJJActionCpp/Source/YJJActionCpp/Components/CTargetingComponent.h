#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CTargetingComponent.generated.h"

class ACCommonCharacter;
class AController;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCTargetingComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION()
		void InputAction_Targeting();

	void Toggle_Target();
	void Begin_Targeting();
	void End_Targeting();
	void ChangeTarget(ACCommonCharacter* InTarget);
	void SetVisibleTargetUI(bool bVisible);
	void Tick_MoveFocusCoolTIme(const float InDelta);
	void Tick_Targeting();
	void ChangeFocus(const bool InRight);

public:
	UPROPERTY(VisibleAnywhere, Category = "Focus")
		bool bTargeting;

	UPROPERTY(EditAnywhere, Category = "Focus")
		float MovingFocus_CurrentCoolTime;

	UPROPERTY(VisibleAnywhere, Category = "Focus")
		float MovingFocus_ConstantTime = 0.3f;

	UPROPERTY(VisibleAnywhere, Category = "Focus")
		bool bMovingFocus;

	UPROPERTY(VisibleAnywhere, Category = "Focus")
		bool bCanMoveFocus;

	UPROPERTY(VisibleAnywhere, Category = "Trace")
		float TraceDistance = 1500.0f;

	UPROPERTY(VisibleAnywhere, Category = "Trace")
		float FinishAngle = 0.1f;

	UPROPERTY(VisibleAnywhere, Category = "Trace")
		float InterpSpeed = 20.0f;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<ACCommonCharacter> Target;
	TWeakObjectPtr<AController> Controller;
};
