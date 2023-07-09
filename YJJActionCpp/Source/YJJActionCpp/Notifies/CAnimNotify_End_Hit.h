#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Commons/CEnums.h"
#include "CAnimNotify_End_Hit.generated.h"

UCLASS()
class YJJACTIONCPP_API UCAnimNotify_End_Hit : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	UPROPERTY(EditAnywhere, Category = "Type")
		CEHitType HitType = CEHitType::Common;
};
