#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Components/CStateComponent.h"
#include "CAnimInstance_Character.generated.h"

class ACCommonCharacter;

UCLASS(Abstract)
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
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		float Speed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		float Pitch;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		float Yaw;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		float Direction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		float Look;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		EStateType StateType = EStateType::Idle;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		EStateType StatePrevType = EStateType::Idle;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		bool bFalling;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		bool bHitting;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "IK")
		bool bFootIK = true;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "IK")
		bool bRidingIK;

protected:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCStateComponent> StateComp;
	FRotator PrevRotation;
};
