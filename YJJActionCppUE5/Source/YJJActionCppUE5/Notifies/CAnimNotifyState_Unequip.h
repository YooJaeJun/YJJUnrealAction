#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CAnimNotifyState_Unequip.generated.h"

// 레거시 /Game/Notifies/ANS_Unequip — UCEquipment::Begin_Unequip / End_Unequip (무기 컴포넌트 경유는 CAnimNotifyState_Equip 와 동일).
UCLASS(meta = (DisplayName = "Unequip"))
class YJJACTIONCPPUE5_API UCAnimNotifyState_Unequip : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
