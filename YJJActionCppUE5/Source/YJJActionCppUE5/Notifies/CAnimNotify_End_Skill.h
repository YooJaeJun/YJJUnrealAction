#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_End_Skill.generated.h"

/** 레거시 `/Game/Notifies/AN_End_Skill` — WeaponComp 스킬 종료 + 스폰 무기 `End_Skill` 디스패치. */
UCLASS(meta = (DisplayName = "End Skill"))
class YJJACTIONCPPUE5_API UCAnimNotify_End_Skill : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere, Category = "Skill", meta = (ClampMin = "0"))
	int32 SkillIndex = 0;
};
