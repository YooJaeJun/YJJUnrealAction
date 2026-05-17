#pragma once

#include "CoreMinimal.h"
#include "CCharacterFeetData.generated.h"

// 레거시 BP 사용자 정의 구조체 FFeetData — 손발·골반 IK 오프셋/회전 스냅샷(스크린샷 순서 그대로).
USTRUCT(BlueprintType)
struct FFeetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Feet")
	FVector LeftFootDistance = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Feet")
	FVector RightFootDistance = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Feet")
	FVector LeftHandDistance = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Feet")
	FVector RightHandDistance = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Feet")
	FVector PelvisDistance = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Feet")
	FRotator LeftFootRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Feet")
	FRotator RightFootRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Feet")
	FRotator LeftHandRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Feet")
	FRotator RightHandRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Feet")
	FRotator PelvisRotation = FRotator::ZeroRotator;
};
