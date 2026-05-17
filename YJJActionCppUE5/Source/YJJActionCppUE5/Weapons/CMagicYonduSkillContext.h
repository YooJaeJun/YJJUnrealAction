#pragma once

#include "CoreMinimal.h"
#include "Weapons/CSkillWeapon.h"
#include "CMagicYonduSkillContext.generated.h"

/**
 * 레거시 Yondu 마법 컨텍스트 흐름 근사 — `ACMagicBombSkillContext` 와 동일하게 DoAction 에서 Idle/Riding 확인 후 PlayAction(0),
 * `Begin_DoAction` 에서 마법 피격체 스폰 후 `Shoot` 호출(Yondu 는 방향 매개변수 없음).
 */
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACMagicYonduSkillContext : public ACMagicSkillContext
{
	GENERATED_BODY()

public:
	/** `/Game/Magics/Yondu/Skill_Yondu` → `ACSkillYondu` 재부모화 후 디폴트 클래스 지정. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Yondu")
	TSubclassOf<class ACSkillYondu> SkillYonduActorClass;

	/** 화살/요르두 발사 근처 — 기본 브런처와 동일(`Hand_Bow_Right_Arrow`)으로 두고 레거시가 다르면 에디터에서 교체. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Yondu")
	FName YonduSpawnSocketName = FName(TEXT("Hand_Bow_Right_Arrow"));

	virtual void DoAction_Implementation(CEAttackType const InAttackType, int32 const InSkillIndex) override;
	virtual void Begin_DoAction_Implementation(CEAttackType const InAttackType) override;
};
