#pragma once

#include "CoreMinimal.h"
#include "CCharacterFeetData.generated.h"

// BP 사용자 정의 구조체 FFeetData 포팅 — 애님/풋 IK용 오프셋·회전 스냅샷.
USTRUCT(BlueprintType)
struct FFeetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet")
	FVector LeftFootDistance = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet")
	FVector RightFootDistance = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet")
	FVector LeftHandDistance = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet")
	FVector RightHandDistance = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet")
	FVector PelvisDistance = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet")
	FRotator LeftFootRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet")
	FRotator RightFootRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet")
	FRotator LeftHandRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet")
	FRotator RightHandRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet")
	FRotator PelvisRotation = FRotator::ZeroRotator;
};
