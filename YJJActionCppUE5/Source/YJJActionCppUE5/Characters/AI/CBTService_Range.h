#pragma once

#include "Characters/AI/CBTService_Character.h"
#include "CBTService_Range.generated.h"

/**
 * 레거시 `/Game/Character/Enemies/Range/BTService_Range` — Magic 과 동일한 Focus·거리 분기,
 * 타겟 무효 시 Avoid(블프 NewEnumerator18) + ClearFocus.
 */
UCLASS(meta = (DisplayName = "BTService Range"))
class YJJACTIONCPPUE5_API UCBTService_Range : public UCBTService_Character
{
	GENERATED_BODY()

public:
	UCBTService_Range();

protected:
	UPROPERTY(EditAnywhere, Category = "Range|Distance", meta = (ClampMin = "1", DisplayName = "Avoid Distance"))
	double AvoidDistance = 500.0;

	UPROPERTY(EditAnywhere, Category = "Range|BB Bytes", meta = (ClampMin = "0"))
	uint8 BehaviorByte_NoTarget = 18;

	UPROPERTY(EditAnywhere, Category = "Range|BB Bytes", meta = (ClampMin = "0"))
	uint8 BehaviorByte_WithinAvoidDistance = 20;

	UPROPERTY(EditAnywhere, Category = "Range|BB Bytes", meta = (ClampMin = "0"))
	uint8 BehaviorByte_BeyondAvoidDistance = 17;

	virtual void TickCharacterAi(UBehaviorTreeComponent& OwnerComp) override;
};
