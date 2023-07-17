#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CCamComponent.generated.h"

class ACCommonCharacter;
class UCTargetingComponent;
class UCMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEnableTopViewCam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDisableTopViewCam);

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
class YJJACTIONCPPUE5_API UCCamComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCCamComponent();

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
	FORCEINLINE void EnableFixedCamera() { bFixedCamera = true; }
	FORCEINLINE void DisableFixedCamera() { bFixedCamera = false; }
	void EnableTopViewCamera() const;
	void DisableTopViewCamera() const;
	FORCEINLINE void SetZooming(const float InZooming) { Zooming = InZooming; }

	FORCEINLINE constexpr float GetZooming() const { return Zooming; }
	FORCEINLINE constexpr bool GetFixedCamera() const { return bFixedCamera; }

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

public:
	FEnableTopViewCam OnEnableTopViewCam;
	FDisableTopViewCam OnDisableTopViewCam;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCTargetingComponent> TargetingComp;
	TWeakObjectPtr<UCMovementComponent> MovementComp;
	bool bFixedCamera = false;
};