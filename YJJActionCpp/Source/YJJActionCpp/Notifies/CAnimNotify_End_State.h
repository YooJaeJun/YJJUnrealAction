#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Component/CStateComponent.h"
#include "CAnimNotify_End_State.generated.h"

UCLASS()
class YJJACTIONCPP_API UCAnimNotify_End_State : public UAnimNotify
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "Type")
		EStateType StateType;

public:
	FString GetNotifyName_Implementation() const override;

	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
