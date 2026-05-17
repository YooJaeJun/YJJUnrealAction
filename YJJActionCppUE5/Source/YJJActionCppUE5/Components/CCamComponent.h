#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CCamComponent.generated.h"

class ACCommonCharacter;
class UCTargetingComponent;
class UCMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEnableTopViewCam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDisableTopViewCam);

USTRUCT(BlueprintType)
struct FZoomData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (ToolTip = "줌 입력(예: 마우스 휠 축)에 따른 변경 속도입니다."))
	float ZoomSpeed = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (ToolTip = "암 길이(줌 거리)의 최소값입니다."))
	float MinRange = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (ToolTip = "암 길이(줌 거리)의 최대값입니다."))
	float MaxRange = 600;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (ToolTip = "목표 암 길이로 보간되는 속도입니다."))
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FZoomData ZoomData;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
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