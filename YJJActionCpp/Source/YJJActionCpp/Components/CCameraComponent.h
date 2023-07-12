#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CCameraComponent.generated.h"

class ACCommonCharacter;
class UCTargetingComponent;
class UCMovementComponent;

USTRUCT()
struct FZoomData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		float ZoomSpeed = 40;

	UPROPERTY(EditAnywhere)
		float MinRange = 100;

	UPROPERTY(EditAnywhere)
		float MaxRange = 600;

	UPROPERTY(EditAnywhere)
		float InterpSpeed = 5;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCCameraComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InputAxis_HorizontalLook(const float InAxis);
	void InputAxis_VerticalLook(const float InAxis);
	void InputAxis_Zoom(const float InAxis);

public:
	void EnableControlRotation() const;
	void DisableControlRotation() const;

public:
	FORCEINLINE void SetZooming(const float InZooming) { Zooming = InZooming; }

	FORCEINLINE constexpr float GetZooming() const { return Zooming; }
	FORCEINLINE constexpr bool GetFixedCamera() const { return bFixedCamera; }
	FORCEINLINE constexpr void FixCamera() { bFixedCamera = true; }
	FORCEINLINE constexpr void UnFixCamera() { bFixedCamera = false; }

private:
	UPROPERTY(EditAnywhere, Category = "Camera")
		float HorizontalLook = 45;

	UPROPERTY(EditAnywhere, Category = "Camera")
		float VerticalLook = 45;

public:
	UPROPERTY(EditAnywhere)
		FZoomData ZoomData;

	UPROPERTY(VisibleAnywhere)
		float Zooming;

	UPROPERTY(VisibleAnywhere)
		float TargetArmLength;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCTargetingComponent> TargetingComp;
	TWeakObjectPtr<UCMovementComponent> MovementComp;
	bool bFixedCamera = false;
};