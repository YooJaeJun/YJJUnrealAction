#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CMovementComponent.generated.h"

class ACCommonCharacter;

UENUM()
enum class ESpeedType : uint8
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
	void EnableControlRotation();
	void DisableControlRotation();

	void SetSpeeds(const TArray<float> InSpeeds);
	void SetSpeed(const ESpeedType Index) const;
	void SetMaxWalkSpeed(const float InSpeed) const;
	void SetWalkSpeed(const float InSpeed) const;
	void SetRunSpeed(const float InSpeed) const;
	void SetSprintSpeed(const float InSpeed) const;
	void SetLerpMove();
	void IsLerpMove() const;

	void SetGravity();
	void AddGravity();

public:
	void InputAxis_MoveForward(const float InAxis);
	void InputAxis_MoveRight(const float InAxis);
	void InputAxis_HorizontalLook(const float InAxis);
	void InputAxis_VerticalLook(const float InAxis);
	void InputAction_Walk();
	void InputAction_Run();
	void InputAction_Jump();

public:
	FORCEINLINE bool CanMove() const { return bCanMove; }
	FORCEINLINE void Move() { bCanMove = true; }
	FORCEINLINE void Stop() { bCanMove = false; }

	FORCEINLINE float GetWalkSpeed() const { return Speeds[(uint8)ESpeedType::Walk]; }
	FORCEINLINE float GetRunSpeed() const { return Speeds[(uint8)ESpeedType::Run]; }
	FORCEINLINE float GetSprintSpeed() const { return Speeds[(uint8)ESpeedType::Sprint]; }

	FORCEINLINE bool GetFixedCamera() const { return bFixedCamera; }
	FORCEINLINE void FixCamera() { bFixedCamera = true; }
	FORCEINLINE void UnFixCamera() { bFixedCamera = false; }

public:	
	UPROPERTY(VisibleAnywhere, Category = "Move")
		bool Moveable;

	UPROPERTY(VisibleAnywhere, Category = "Move")
		bool CameraFixed;
	
	UPROPERTY(VisibleAnywhere, Category = "Move")
		float Speeds[static_cast<uint8>(ESpeedType::Max)] = { 200, 500, 800 };

	UPROPERTY(VisibleAnywhere, Category = "Lerp")
		bool LerpMoving;

	UPROPERTY(VisibleAnywhere, Category = "Lerp")
		float LerpMoveDistance;

	UPROPERTY(VisibleAnywhere, Category = "Lerp")
		FVector Dest;

	UPROPERTY(EditAnywhere, Category = "Lerp")
		float InterpSpeed = 5;

private:
	UPROPERTY(EditAnywhere, Category = "Camera")
		float HorizontalLook = 45;

	UPROPERTY(EditAnywhere, Category = "Camera")
		float VerticalLook = 45;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	bool bCanMove = true;
	bool bFixedCamera;
};
