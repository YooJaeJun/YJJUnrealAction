#pragma once
#include "CoreMinimal.h"
#include "Weapons/CSkill.h"
#include "Components/TimelineComponent.h"
#include "CSkill_Aiming.generated.h"

class UCurveVector;
class ACCommonCharacter;
class ACAttachment;
class UCAct;
class USpringArmComponent;
class UCameraComponent;

// Bow Skill_Aim 의 FAimData(Weapons/Bow/FAimData.h) 와 동일 이름이면 UHT 가 둘 다 "AimData" 로 처리해 충돌한다.
USTRUCT()
struct FAimingSkillSnapData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Zoom")
	float TargetArmLength = 100;

	UPROPERTY(EditAnywhere)
	FVector SocketOffset = FVector(0, 30, 10);

	UPROPERTY(EditAnywhere)
	bool bEnableCameraLag;

	UPROPERTY(EditAnywhere)
	FVector CameraLocation;
};

UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API UCSkill_Aiming : public UCSkill
{
	GENERATED_BODY()


public:
	UCSkill_Aiming();

public:
	virtual void BeginPlay(
		TWeakObjectPtr<ACCommonCharacter> InOwner,
		ACAttachment* InAttachment,
		UCAct* InAct) override;

	virtual void Tick_Implementation(float InDeltaTime) override;

public:
	virtual void Pressed() override;
	virtual void Released() override;

private:
	UFUNCTION()
	void OnAiming(FVector Output);

private:
	UPROPERTY(EditAnywhere, Category = "Aiming")
	TObjectPtr<UCurveVector> Curve;

	UPROPERTY(EditAnywhere, Category = "Aiming")
	FAimingSkillSnapData AimData;

	UPROPERTY(EditAnywhere, Category = "Aiming")
	float AimingSpeed = 200;

private:
	TWeakObjectPtr<USpringArmComponent> SpringArm;
	TWeakObjectPtr<UCameraComponent> Camera;
	FTimeline Timeline;
	FAimingSkillSnapData OriginData;
	TSharedPtr<float> Bend;
};
