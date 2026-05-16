#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_Move.generated.h"

// 레거시 /Game/Notifies/AN_Move — 소유 액터의 UCMovementComponent::Move() 호출(Gait 등에서 입력 이동 허용 bCanMove = true).
UCLASS(meta = (DisplayName = "Move"))
class YJJACTIONCPPUE5_API UCAnimNotify_Move : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
