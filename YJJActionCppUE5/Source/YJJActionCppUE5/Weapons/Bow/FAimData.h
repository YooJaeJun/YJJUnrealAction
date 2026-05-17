#pragma once

#include "CoreMinimal.h"
#include "FAimData.generated.h"

/**
 * 레거시 `/Game/Weapons/Bow/FAimData` UserDefinedStruct 와 필드 순서를 맞춘다.
 * 활 조준 카메라(SpringArm·Camera)·줌용 스냅샷 저장에 사용된다.
 */
USTRUCT(BlueprintType)
struct FAimData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	float TargetArmLength = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	FVector SocketOffset = FVector(0.f, 30.f, 10.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	FVector TargetOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	bool bEnableCameraLag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	FVector CameraLocation = FVector::ZeroVector;
};
