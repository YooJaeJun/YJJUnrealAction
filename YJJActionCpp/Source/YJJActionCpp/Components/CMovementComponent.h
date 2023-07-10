#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CMovementComponent.generated.h"

class ACCommonCharacter;
class UCStateComponent;

UENUM()
enum class CESpeedType : uint8
{
	Walk,
	Run,
	Sprint,
	Max
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCMovementComponent();

protected:
	virtual void BeginPlay() override;

public:
	void EnableControlRotation() const;
	void DisableControlRotation() const;

	void SetSpeeds(const TArray<float> InSpeeds);
	void SetSpeed(const CESpeedType Index) const;
	void SetMaxWalkSpeed(const float InSpeed) const;
	void SetWalkSpeed() const;
	void SetRunSpeed() const;
	void SetSprintSpeed() const;
	void SetLerpMove() const;
	void IsLerpMove() const;

	void SetGravity(const float InValue) const;
	void AddGravity(const float InValue) const;

	void SetFriction(const float InFriction, const float InBraking) const;
	void SetJumpZ(const float InVelocity) const;

public:
	void InputAxis_MoveForward(const float InAxis);
	void InputAxis_MoveRight(const float InAxis);
	void InputAxis_HorizontalLook(const float InAxis);
	void InputAxis_VerticalLook(const float InAxis);
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

	FORCEINLINE constexpr bool GetFixedCamera() const { return bFixedCamera; }
	FORCEINLINE constexpr void FixCamera() { bFixedCamera = true; }
	FORCEINLINE constexpr void UnFixCamera() { bFixedCamera = false; }

public:	
	UPROPERTY(VisibleAnywhere, Category = "Move")
		bool CameraFixed;
	
	UPROPERTY(VisibleAnywhere, Category = "Move")
		float Speeds[static_cast<uint8>(CESpeedType::Max)] = { 200, 500, 800 };

	UPROPERTY(VisibleAnywhere, Category = "Lerp")
		bool LerpMoving;

	UPROPERTY(VisibleAnywhere, Category = "Lerp")
		float LerpMoveDistance;

	UPROPERTY(VisibleAnywhere, Category = "Lerp")
		FVector Dest;

	UPROPERTY(EditAnywhere, Category = "Lerp")
		float InterpSpeed = 5;

	UPROPERTY(EditAnywhere, Category = "Animation")
		float Forward;

	UPROPERTY(EditAnywhere, Category = "Animation")
		float Right;

	UPROPERTY(EditAnywhere, Category = "Animation")
		float SpeedFactor = 4;

private:
	UPROPERTY(EditAnywhere, Category = "Camera")
		float HorizontalLook = 45;

	UPROPERTY(EditAnywhere, Category = "Camera")
		float VerticalLook = 45;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCStateComponent> StateComp;
	bool bCanMove = true;
	bool bFixedCamera = false;
};
