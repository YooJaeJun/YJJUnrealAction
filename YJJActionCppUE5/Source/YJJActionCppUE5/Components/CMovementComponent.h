#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Commons/CEnums.h"
#include "CMovementComponent.generated.h"

class ACCommonCharacter;
class UCStateComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPPUE5_API UCMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCMovementComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	void OnEnableTopViewCam();

	UFUNCTION()
	void OnDisableTopViewCam();

public:
	void SetSpeeds(const TArray<float> InSpeeds);
	void SetSpeed(const CESpeedType Index) const;
	void SetMaxWalkSpeed(const float InSpeed) const;
	void SetWalkSpeed() const;
	void SetRunSpeed() const;
	void SetSprintSpeed() const;

	void SetGravity(const float InValue) const;
	void AddGravity(const float InValue) const;

	void SetFriction(const float InFriction, const float InBraking) const;
	void SetJumpZ(const float InVelocity) const;

public:
	void InputAxis_MoveForward(const float InAxis);
	void InputAxis_MoveRight(const float InAxis);
	void InputAction_Walk();
	void InputAction_Run();
	void InputAction_Jump();

public:
	bool CanMove(const float InAxis) const;
	FORCEINLINE bool CanMove() const { return bCanMove; }
	FORCEINLINE void Move() { bCanMove = true; }
	FORCEINLINE void Stop() { bCanMove = false; }

	FORCEINLINE constexpr float GetWalkSpeed() const { return Speeds[static_cast<uint8>(CESpeedType::Walk)]; }
	FORCEINLINE constexpr float GetRunSpeed() const { return Speeds[static_cast<uint8>(CESpeedType::Run)]; }
	FORCEINLINE constexpr float GetSprintSpeed() const { return Speeds[static_cast<uint8>(CESpeedType::Sprint)]; }

	// 레거시 BP MovingComponent::Is Can Move — 점프 등에서 이동 가능 여부만 조회할 때 사용.
	UFUNCTION(BlueprintPure, Category = "Move", meta = (DisplayName = "Is Can Move"))
	bool IsCanMove() const;

	// 레거시 BP MovingComponent::Get Fixed Camera — 실제 값은 소유자의 CamComponent 에 있다.
	UFUNCTION(BlueprintPure, Category = "Move")
	bool GetFixedCamera() const;

	// BP MovingComponent — 월드 위치 Lerp 이동 중인지 (Tick_LerpMove 게이트).
	UFUNCTION(BlueprintPure, Category = "Move", meta = (DisplayName = "Is Lerp Move"))
	bool IsLerpMove() const { return bLerpMove; }

	UFUNCTION(BlueprintCallable, Category = "Move", meta = (DisplayName = "Set Lerp Move"))
	void SetLerpMove(const bool bIn) { bLerpMove = bIn; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Lerp", meta = (DisplayName = "Dest"))
	FVector Dest = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move|Lerp", meta = (DisplayName = "Interp Speed"))
	float InterpSpeed = 8.f;

public:
	UPROPERTY(EditAnywhere, Category = "Move")
	float Speeds[static_cast<uint8>(CESpeedType::Max)] = { 200, 500, 800 };

	UPROPERTY(EditAnywhere, Category = "Animation")
	float Forward;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float Right;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float SpeedFactor = 4;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCStateComponent> StateComp;
	bool bCanMove = true;
	bool bTopViewCam = false;

	// BP MovingComponent::LerpMoving
	bool bLerpMove = false;
};
