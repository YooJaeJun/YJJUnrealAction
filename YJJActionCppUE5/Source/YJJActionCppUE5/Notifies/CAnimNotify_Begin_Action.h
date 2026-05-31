#pragma once

#include "CoreMinimal.h"
#include "Commons/CEnums.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_Begin_Action.generated.h"

// 레거시 /Game/Notifies/AN_Begin_Action — Magic(IsUnarmed) 분기 후 WeaponComponent::Begin_DoAction.
UCLASS(meta = (DisplayName = "Begin Action"))
class YJJACTIONCPPUE5_API UCAnimNotify_Begin_Action : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Begin Action")
	CEAttackType AttackType = CEAttackType::Common;

	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
