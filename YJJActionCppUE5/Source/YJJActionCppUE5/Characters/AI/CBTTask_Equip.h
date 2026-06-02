#pragma once

#include "CoreMinimal.h"
#include "Characters/AI/CBTTask_Custom.h"
#include "Commons/CEnums.h"
#include "CBTTask_Equip.generated.h"

/** 레거시 BTTask_Equip — WeaponType/MagicType 장착 후 IsEquipping·Idle 종료까지 Tick 대기. */
UCLASS(DisplayName = "BTTask Equip")
class YJJACTIONCPPUE5_API UCBTTask_Equip : public UCBTTask_Custom
{
	GENERATED_BODY()

public:
	UCBTTask_Equip();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Equip", meta = (DisplayName = "Weapon Type"))
	CEWeaponType WeaponType = CEWeaponType::Unarmed;

	UPROPERTY(EditAnywhere, Category = "Equip", meta = (DisplayName = "Magic Type"))
	CEMagicType MagicType = CEMagicType::Unarmed;
};
