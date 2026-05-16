#pragma once

#include "CoreMinimal.h"
#include "Commons/CEnums.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_End_Action.generated.h"

// 레거시 /Game/Notifies/AN_End_Action — Magic 슬롯·무장 여부에 따라 UCMagicComponent::End_DoAction 또는 UCWeaponComponent::End_DoAction 호출.
UCLASS(meta = (DisplayName = "End Action"))
class YJJACTIONCPPUE5_API UCAnimNotify_End_Action : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "End Action")
	CEAttackType AttackType = CEAttackType::Common;

	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
