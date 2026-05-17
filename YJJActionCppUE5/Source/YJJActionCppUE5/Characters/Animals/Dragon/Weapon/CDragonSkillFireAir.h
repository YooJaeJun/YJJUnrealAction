#pragma once

#include "CoreMinimal.h"
#include "Characters/Animals/Dragon/Weapon/CDragonSkillFire.h"
#include "CDragonSkillFireAir.generated.h"

/**
 * 레거시 `/Game/Character/Animals/Dragon/Weapon/FireAir/Skill_Dragon_FireAir`.
 *
 * 바닥용 `ACSkillDragonFire` 대비:
 * - 틱(Attached=true): FireEffect 에 `ECollisionEnabled::QueryAndPhysics` 를 켠다(`Skill_Dragon_FireAir` Tick 합성).
 * - Fire: 루프 사운드 직후 **같은 액터 위치** 에서 `ShootSound` 한 번 재생 후 `Attached` (`Skill_Dragon_FireAir:Fire`).
 * - End(DisplayName 그대로 부모 제공): 숨김 → `BreathEmitterToggleName`(보통 Fire) 끔 → `BreathSmokeEmitterToggleName`(기본 Smoke) 끔 (`Skill_Dragon_FireAir:End`).
 *
 * 레거시 `BeginPlay`(Dragon 의 깨진 IsDead 디스패처 등) 및 겹침 BF_Helpers 는 `ACSkillDragonFire` 로 이미 대체되었으므로 BP 에서 부모 교체 후 그래프는 비우면 된다.
 */
UCLASS(
	Blueprintable,
	meta=(
		DisplayName = "Dragon Skill — Fire Breath (Air)",
		BlueprintDescription = "Skill_Dragon_FireAir 네이티브 버전입니다. 레거시 Tick/Fire/End 합성은 본 클래스가 담당합니다."))
class YJJACTIONCPPUE5_API ACSkillDragonFireAir : public ACSkillDragonFire
{
	GENERATED_BODY()

public:
	ACSkillDragonFireAir();

	/** 레거시 `End`: 두 번째 `SetEmitterEnable` 대상(기본 `Smoke`). 파티클 에셋에 없으면 엔진이 무시하는 수준이다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon|Skill|FireAir|VFX")
	FName BreathSmokeEmitterToggleName = FName(TEXT("Smoke"));

protected:
	virtual void DragonFire_OnAfterBaselineFireScratch() override;

	virtual void DragonFire_ApplyEndVfxScratch() override;

	virtual void DragonFire_OnAttachedBreathAfterTransformScratch() override;
};
