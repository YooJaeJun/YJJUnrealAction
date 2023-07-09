#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Components/CStateComponent.h"
#include "CAnimNotify_End_State.generated.h"

UCLASS()
class YJJACTIONCPP_API UCAnimNotify_End_State : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	UPROPERTY(EditAnywhere, Category = "Type")
		CEStateType StateType;
};
