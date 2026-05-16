#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimMontage.h"
#include "Engine/DataTable.h"
#include "Commons/CEnums.h"
#include "FParkourData.generated.h"

/**
 * 블루프린트 UDS FParkourData 의 C++ 버전이다.
 * `/Game/Parkour/ParkourData` 같은 데이터 테이블의 Row 구조를 에디터에서 이 타입으로 지정해야 한다.
 */
USTRUCT(BlueprintType)
struct YJJACTIONCPPUE5_API FParkourData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	CEParkourType Type = CEParkourType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	float PlayRatio = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	FName Section;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	float MinDistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	float MaxDistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	float Extent = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour", meta = (DisplayName = "Fixed Camera"))
	bool bFixedCamera = false;
};
