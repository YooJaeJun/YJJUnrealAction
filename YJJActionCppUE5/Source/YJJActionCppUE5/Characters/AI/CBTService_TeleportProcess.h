#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Commons/CEnums.h"
#include "CBTService_TeleportProcess.generated.h"

class ACDragon;
class AAIController;

/**
 * 레거시 `/Game/Character/Animals/Dragon/AI/BTService_TeleportProcess` — ReceiveActivationAI 에서
 * DragonWeapon.TeleportProcessType 을 캐시하고, ReceiveTickAI 에서 블랙보드·StateTextRender 를 갱신한다.
 */
UCLASS(meta = (DisplayName = "BT Service (Teleport Process)"))
class YJJACTIONCPPUE5_API UCBTService_TeleportProcess : public UBTService
{
	GENERATED_BODY()

public:
	UCBTService_TeleportProcess();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "BB", meta = (DisplayName = "Teleport Process Key"))
	FBlackboardKeySelector TeleportProcessKey;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bUpdateStateTextRender = true;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "세팅",
		meta = (AllowPrivateAccess = "true", DisplayName = "Controller"))
	TObjectPtr<AAIController> OwningAi;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "세팅",
		meta = (AllowPrivateAccess = "true", DisplayName = "Dragon"))
	TObjectPtr<ACDragon> CachedDragon;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "세팅",
		meta = (AllowPrivateAccess = "true", DisplayName = "Teleport Process"))
	CETeleportProcessType TeleportProcess = CETeleportProcessType::None;

private:
	void RefreshDragonWeaponTeleportProcess();

	void PushTeleportProcess(UBehaviorTreeComponent& OwnerComp);

	void WriteBlackboardTeleportProcess(UBehaviorTreeComponent& OwnerComp, uint8 InProcessByte) const;

	void TryWriteStateDebugText(ACDragon* TextSourceDragon, CETeleportProcessType InProcess) const;
};
