#pragma once

#include "CoreMinimal.h"
#include "Weapons/CSkillWeapon.h"
#include "CDragonSkillMagic.generated.h"

class ACDragon;
class ACDragonWeapon;

/**
 * 레거시 `/Game/Character/Animals/Dragon/Weapon/Skill_Dragon` (Skill_Magic 파생).
 * BP_Dragon_AI 캐스트·숨김 충돌용 Character 중복 변수를 제거하고, BeginPlay 에서 소유(ACDragon 또는 ACDragonWeapon)·무기 레퍼런스 및
 * 투사/충돌 우회용 `Actors To Ignore` 기본 채우기를 재현한다.
 *
 * 블루프린트는 클래스 부모를 `ACSkillDragon` 로 바꾸고 에디터에 남아 있던 레거시 Dragon/무기 변수 그래프는 제거하면 된다.
 */
UCLASS(
	Blueprintable,
	meta=(
		DisplayName = "Dragon Skill Magic Actor",
		BlueprintDescription = "레거시 Skill_Magic 블프 Skill_Dragon 전용 부모입니다. Owner 는 ACDragon 또는 ACDragonWeapon 로 두면 ACSkillWeapon::BeginPlay 에서 Character 가 채워집니다."))
class YJJACTIONCPPUE5_API ACSkillDragon : public ACSkillMagic
{
	GENERATED_BODY()

public:
	ACSkillDragon();

protected:
	virtual void BeginPlay() override;

public:
	/** 레거시 `Dragon` 변수 — 블루프린트/충돌에서 드래곤 폰 액터를 직접 참조한다. */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Dragon|Skill",
		meta = (MultiLine = "true", DisplayName = "Dragon"))
	TObjectPtr<ACDragon> Dragon;

	/** 레거시 `Dragon Weapon` — Owner 가 무기이면 매칭하고, 폰만 알려진 경우 폰의 DragonWeapon 을 따라간다. */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Dragon|Skill",
		meta = (MultiLine = "true", DisplayName = "Dragon Weapon"))
	TObjectPtr<ACDragonWeapon> DragonWeapon;

	/** 투사/트레이스 등에서 무시 목록(bl프 배열 순서 유지 의도: 스킬 본체 → 드래곤 → (선택) 상호작용 액터). */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Dragon|Skill",
		meta = (MultiLine = "true", DisplayName = "Actors To Ignore"))
	TArray<TObjectPtr<AActor>> ActorsToIgnore;

	/** BeginPlay 에서 레거시와 같이 스킬·드래곤·CurInteractingActor 를 ActorsToIgnore 에 넣는다. 디버깅 위해 끌 수 있다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon|Skill")
	bool bAppendDefaultActorsToIgnoreOnBeginPlay = true;

private:
	static void SkillDragon_AppendUniqueIgnoreActor(TArray<TObjectPtr<AActor>>& IgnoreListScratch, AActor* CandidateScratch);

	void SkillDragon_RebuildDragonRefsFromOwnerAndCharacter();
};
