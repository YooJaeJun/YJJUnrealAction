#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Components/CStateComponent.h"
#include "CAnimInstance_Character.generated.h"

class ACCommonCharacter;
class UCMovementComponent;
class UCFlyComponent;

UCLASS(Abstract)
class YJJACTIONCPP_API UCAnimInstance_Character : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UFUNCTION()
		void OnStateTypeChanged(const CEStateType InPrevType, const CEStateType InNewType);

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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		float Forward;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		float Side;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		CEStateType StateType = CEStateType::Idle;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "States")
		CEStateType StatePrevType = CEStateType::Idle;

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
	TWeakObjectPtr<UCMovementComponent> MovementComp;
	TWeakObjectPtr<UCFlyComponent> FlyComp;
	FRotator PrevRotation;
};
