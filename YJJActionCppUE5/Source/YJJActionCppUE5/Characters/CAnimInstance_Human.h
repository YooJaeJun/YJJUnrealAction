#pragma once

#include "CoreMinimal.h"
#include "Characters/CAnimInstance_Character.h"
#include "CAnimInstance_Human.generated.h"

class UCharacterMovementComponent;

// 레거시 /Game/Character/CABP_Human — UCAnimInstance_Human 필수 이동 데이터만 네이티브로 채운다.
UCLASS()
class YJJACTIONCPPUE5_API UCAnimInstance_Human : public UCAnimInstance_Character
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** 레거시 CABP References::Movement Component — 그래프 Set 노드 허용(값은 매 틱 Native 업데이트로 덮어씀 가능). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "References",
		meta = (AllowPrivateAccess = "true", DisplayName = "Movement Component"))
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Essential Movement Data")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Essential Movement Data")
	double GroundSpeed = 0.0;

	UPROPERTY(
		BlueprintReadWrite,
		EditAnywhere,
		Category = "Essential Movement Data",
		meta = (DisplayName = "Should Move"))
	bool ShouldMove = false;

	UPROPERTY(
		BlueprintReadWrite,
		EditAnywhere,
		Category = "Essential Movement Data",
		meta = (DisplayName = "Is Falling"))
	bool IsFalling = false;

private:
	void ResolveMovementComponent();

	/** BP Greater_DoubleDouble B 핀 기본값 3 과 동일(미세 속도 스팸 차단). */
	static constexpr double ShouldMoveGroundSpeedEpsilon = 3.0;
};
 