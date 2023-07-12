#pragma once
#include "CoreMinimal.h"
#include "Weapons/CSkill.h"
#include "Weapons/CWeaponStructures.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CSkill_Common.generated.h"

UCLASS(Blueprintable)
class YJJACTIONCPP_API UCSkill_Common : public UCSkill
{
	GENERATED_BODY()

public:
    virtual void Pressed() override;
    virtual void Begin_Skill_Implementation() override;
    virtual void End_Skill_Implementation() override;

private:
    UFUNCTION()
        void OnAttachmentBeginOverlap(
            ACCommonCharacter* InAttacker,
            AActor* InAttackCauser,
            ACCommonCharacter* InOther);

private:
    UPROPERTY(EditDefaultsOnly, Category = "Action")
		FActData ActData;

	UPROPERTY(EditDefaultsOnly, Category = "Action")
		FHitData HitData;

private:
	TArray<TWeakObjectPtr<ACCommonCharacter>> Overlapped;
	TArray<TWeakObjectPtr<ACCommonCharacter>> Hitted;
};