#pragma once

#include "Characters/AI/CBTService_Character.h"
#include "CBTService_Magic.generated.h"

/**
 * 레거시 `/Game/Character/Enemies/Magic/BTService_Magic` — 타겟 유효 시 Focus·거리별 Behavior,
 * 무효 시 Patrol(블프 NewEnumerator14) + ClearFocus.
 */
UCLASS(meta = (DisplayName = "BTService Magic"))
class YJJACTIONCPPUE5_API UCBTService_Magic : public UCBTService_Character
{
	GENERATED_BODY()

public:
	UCBTService_Magic();

protected:
	UPROPERTY(EditAnywhere, Category = "Magic|Distance", meta = (ClampMin = "1", DisplayName = "Avoid Distance"))
	double AvoidDistance = 500.0;

	UPROPERTY(EditAnywhere, Category = "Magic|BB Bytes", meta = (ClampMin = "0"))
	uint8 BehaviorByte_NoTarget = 14;

	UPROPERTY(EditAnywhere, Category = "Magic|BB Bytes", meta = (ClampMin = "0"))
	uint8 BehaviorByte_WithinAvoidDistance = 20;

	UPROPERTY(EditAnywhere, Category = "Magic|BB Bytes", meta = (ClampMin = "0"))
	uint8 BehaviorByte_BeyondAvoidDistance = 17;

	virtual void TickCharacterAi(UBehaviorTreeComponent& OwnerComp) override;
};
