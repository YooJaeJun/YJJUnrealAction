#pragma once
#include "CoreMinimal.h"
#include "Weapons/CSkill.h"
#include "CSkill_Warp.generated.h"

class ACameraActor;
class ACCommonCharacter;
class APlayerController;
class ACameraActor;

UCLASS(Blueprintable)
class YJJACTIONCPP_API UCSkill_Warp : public UCSkill
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		TSubclassOf<ACameraActor> CameraActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		FVector CameraRelativeLocation = FVector(0, 0, 1000);

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		TEnumAsByte<ECameraProjectionMode::Type> ProjectionMode;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		float OrthoWidth = 2000;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		float FieldOfView = 90;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		float BlendIn = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		float BlendOut = 0;

public:
	UCSkill_Warp();

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
	TWeakObjectPtr<APlayerController> Controller;
	TWeakObjectPtr<ACameraActor> CameraActor;
};
