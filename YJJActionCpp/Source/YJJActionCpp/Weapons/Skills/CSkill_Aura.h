#pragma once
#include "CoreMinimal.h"
#include "Weapons/CSkill.h"
#include "Weapons/CWeaponStructures.h"
#include "CSkill_Aura.generated.h"

class ACSkillCollider_Aura;
struct FActData;

UCLASS(Blueprintable)
class YJJACTIONCPP_API UCSkill_Aura : public UCSkill
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category = "Aura")
		TSubclassOf<ACSkillCollider_Aura> AuraClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aura")
		FVector AuraLocation;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Act")
		FActData ActData;

public:
	void Pressed() override;

	void Begin_Skill_Implementation() override;
	void End_Skill_Implementation() override;
};
