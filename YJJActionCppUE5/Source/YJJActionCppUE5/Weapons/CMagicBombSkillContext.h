#pragma once

#include "CoreMinimal.h"
#include "Weapons/CSkillWeapon.h"
#include "CMagicBombSkillContext.generated.h"

/**
 * 레거시 `/Game/Magics/Bomb/Magic_Bomb` — Idle/Riding 및 DoActionDatas 가 있으면 SetAction 후 PlayAction(0),
 * Begin_DoAction 에서 소켓 기준 폭발 액터 스폰 후 Shoot 호출(AI 의 경우 블랙보드 Target 방향).
 */
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACMagicBombSkillContext : public ACMagicSkillContext
{
	GENERATED_BODY()

public:
	/** 레거시 BPVar `Direction`(BlueprintPrivate)·런타임 조준 방향 스냅샷 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Magic|Bomb",
		meta = (MultiLine = "true", DisplayName = "Direction"))
	FVector BombDirection = FVector::ZeroVector;

	/** 레거시 BPVar `Location` — 스폰 위치 스냅샷 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Magic|Bomb",
		meta = (MultiLine = "true", DisplayName = "Location"))
	FVector BombSpawnLocation = FVector::ZeroVector;

	/**
	 * `/Game/Magics/Bomb/Skill_Bomb` → `ACSkillBomb` 재부모화 후 디폴트 지정 — 비면 스폰 생략.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Bomb")
	TSubclassOf<class ACSkillBomb> SkillBombActorClass;

	virtual void DoAction_Implementation(CEAttackType InAttackType, int32 InSkillIndex) override;
	virtual void Begin_DoAction_Implementation(CEAttackType InAttackType) override;

private:
	/** 레거시 `Target` 블랙보드 오브젝트(플레이어 컨트롤러 무효 시 적 AI 폴백) */
	static const FName BombAiBlackboardTargetKey;

	static constexpr float BombBlackboardNormalizeTolerance = 0.001f;

	/** 레거시 `Hand_FireBall` 소켓 */
	static const FName BombSpawnSocketName;

	bool Bomb_TryResolveDirectionFromBlackboard(FVector& OutDirection) const;
};
