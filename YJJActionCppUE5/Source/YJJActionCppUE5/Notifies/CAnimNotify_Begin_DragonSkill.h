#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Commons/CEnums.h"
#include "CAnimNotify_Begin_DragonSkill.generated.h"

/**
 * 레거시 `/Game/Notifies/Dragon/AN_Begin_DragonSkill`.
 * 소유 폰(ACDragon)·`DragonWeapon`·`CEDragonBossSkillType`(노티 프로퍼티 `SkillType`)을 따라 `Begin_Skill` 호출한다.
 */
UCLASS(meta = (DisplayName = "Begin Dragon Skill"))
class YJJACTIONCPPUE5_API UCAnimNotify_Begin_DragonSkill : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Dragon",
		meta = (DisplayName = "Skill Type"))
	CEDragonBossSkillType SkillType = CEDragonBossSkillType::Roar;

	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
