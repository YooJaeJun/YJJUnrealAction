#pragma once

#include "Characters/AI/CBTService_Character.h"
#include "CBTService_Melee.generated.h"

/**
 * 레거시 `/Game/Character/Enemies/Melee/BTService_Melee` — 근거리 Action·Idle/Rise 접근·
 * 타겟 없음 시 Pawn 스피어 트레이스 후 SetTarget / Patrol 분기.
 */
UCLASS(meta = (DisplayName = "BTService Melee"))
class YJJACTIONCPPUE5_API UCBTService_Melee : public UCBTService_Character
{
	GENERATED_BODY()

public:
	UCBTService_Melee();

protected:
	UPROPERTY(EditAnywhere, Category = "Melee|Distance", meta = (ClampMin = "1", DisplayName = "Action Distance"))
	double ActionDistance = 200.0;

	UPROPERTY(EditAnywhere, Category = "Melee|BB Bytes", meta = (ClampMin = "0"))
	uint8 BehaviorByte_InMeleeRange = 17;

	UPROPERTY(EditAnywhere, Category = "Melee|BB Bytes", meta = (ClampMin = "0"))
	uint8 BehaviorByte_OutOfRangeIdleOrRise = 11;

	UPROPERTY(EditAnywhere, Category = "Melee|BB Bytes", meta = (ClampMin = "0"))
	uint8 BehaviorByte_TraceMissed = 18;

	virtual void TickCharacterAi(UBehaviorTreeComponent& OwnerComp) override;
};
