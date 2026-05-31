#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_Bound.generated.h"

// 레거시 /Game/Notifies/AN_Begin_Bound — Mesh Owner(BP_Player/BP_Enemy) 의 Begin_Bound 호출.
UCLASS(meta = (DisplayName = "Begin Bound"))
class YJJACTIONCPPUE5_API UCAnimNotify_Begin_Bound : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};

// 레거시 /Game/Notifies/AN_End_Bound — Mesh Owner 의 End_Bound 호출.
UCLASS(meta = (DisplayName = "End Bound"))
class YJJACTIONCPPUE5_API UCAnimNotify_End_Bound : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
