#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Characters/CCharacterFeetData.h"
#include "CFeetComponent.generated.h"

// ABP_Character 가 BP FeetComponent.Data 를 읽던 경로의 네이티브 대응 — 풋 IK 스냅샷을 애님에서 복사한다.
UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class YJJACTIONCPPUE5_API UCFeetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCFeetComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feet")
	FFeetData Data;
};
