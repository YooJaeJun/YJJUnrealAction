#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_End_Parkour.generated.h"

/**
 * 레거시 `/Game/Notifies/AN_End_Parkour` 와 동일: 스켈레탈 소유자의 CParkourComponent::End_DoParkour 호출.
 * 몽타주 종료 시 파쿠르 상태·충돌 복귀 처리에 사용된다.
 */
UCLASS()
class YJJACTIONCPPUE5_API UCAnimNotify_End_Parkour final : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
