#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Components/CStateComponent.h"
#include "CAnimInstance_Character.generated.h"

class ACCommonCharacter;

UCLASS()
class YJJACTIONCPP_API UCAnimInstance_Character : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UFUNCTION()
		void OnStateTypeChanged(const EStateType InPrevType, const EStateType InNewType);

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
		float Speed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
		float Pitch;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
		float Direction;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		EStateType StateType = EStateType::Idle;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		EStateType StatePrevType = EStateType::Idle;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		bool Falling;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		bool Hitting;

protected:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCStateComponent> StateComp;
	FRotator PrevRotation;
};
