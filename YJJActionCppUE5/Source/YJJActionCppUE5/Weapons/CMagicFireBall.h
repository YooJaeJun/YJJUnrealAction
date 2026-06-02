#pragma once

#include "CoreMinimal.h"

#include "Weapons/CSkillWeapon.h"

#include "CMagicFireBall.generated.h"

/**
 * 레거시 FireBall 마법 컨텍스트 — `CMagicBomb` 과 동형으로 Idle/Riding·DoActionRows 확인 후 PlayAction,
 * `Begin_DoAction` 에서 `Hand_FireBall` 소켓에 투사체 스폰 후 `Shoot`(컨트롤러 또는 블랙보드 Target 방향).
 */
UCLASS(Blueprintable, meta=(DisplayName="CMagicFireBall"))
class YJJACTIONCPPUE5_API ACMagicFireBall : public ACMagic
{
	GENERATED_BODY()

public:
	/** 레거시 `Direction` 근처 — 실행 직전에 컨텍스트가 채운다(Begin_DoAction). */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Magic|FireBall",
		meta = (DisplayName = "Direction"))
	FVector FireBallDirectionScratch = FVector::ZeroVector;

	/** `/Game/Magics/FireBall/Skill_FireBall` → `ACSkillFireBall` 재부모 후 지정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|FireBall")
	TSubclassOf<class ACSkillFireBall> SkillFireBallActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|FireBall")
	FName FireBallSpawnSocketName = FName(TEXT("Hand_FireBall"));

	virtual void DoAction_Implementation(CEAttackType const InAttackType, int32 const InSkillIndex) override;

	virtual void Begin_DoAction_Implementation(CEAttackType const InAttackType) override;

private:
	static const FName FireBallAiBlackboardTargetKey;

	static constexpr float FireBallBlackboardNormalizeToleranceScratch = 0.001f;

	bool FireBall_TryResolveDirectionFromBlackboardScratch(FVector& OutDirectionScratch) const;
};
