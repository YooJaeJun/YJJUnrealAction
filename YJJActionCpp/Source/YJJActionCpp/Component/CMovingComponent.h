#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CMovingComponent.generated.h"

class ACCharacter;

UENUM()
enum class ESpeedType : uint8
{
	Walk,
	Run,
	Sprint,
	Max
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCMovingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCMovingComponent();

	UPROPERTY(VisibleAnywhere, Category = "Move")
		bool Moveable;

	UPROPERTY(VisibleAnywhere, Category = "Move")
		bool CameraFixed;
	
	UPROPERTY(VisibleAnywhere, Category = "Move")
		float Speeds[(uint8)ESpeedType::Max] = { 200, 500, 800 };

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

public:
	FORCEINLINE bool CanMove() { return bCanMove; }
	FORCEINLINE void Move() { bCanMove = true; }
	FORCEINLINE void Stop() { bCanMove = false; }

	FORCEINLINE float GetWalkSpeed() { return Speeds[(uint8)ESpeedType::Walk]; }
	FORCEINLINE float GetRunSpeed() { return Speeds[(uint8)ESpeedType::Run]; }
	FORCEINLINE float GetSprintSpeed() { return Speeds[(uint8)ESpeedType::Sprint]; }

	FORCEINLINE bool GetFixedCamera() { return bFixedCamera; }
	FORCEINLINE void FixCamera() { bFixedCamera = true; }
	FORCEINLINE void UnFixCamera() { bFixedCamera = false; }

protected:
	virtual void BeginPlay() override;

public:
	void EnableControlRotation();
	void DisableControlRotation();

	void SetSpeeds(const TArray<float> InSpeeds);
	void SetMaxWalkSpeed(const ESpeedType Index);
	void SetLerpMove();
	void IsLerpMove();

	void SetGravity();
	void AddGravity();

private:
	TWeakObjectPtr<ACCharacter> Owner;
	bool bCanMove = true;
	bool bFixedCamera;
};
