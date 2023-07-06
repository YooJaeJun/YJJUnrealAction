#pragma once
#include "CoreMinimal.h"
#include "Weapons/CSkill.h"
#include "Weapons/CWeaponStructures.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CSkill_Thrust.generated.h"

class ACMotionTrail;
class ACCommonCharacter;

UCLASS(Blueprintable)
class YJJACTIONCPP_API UCSkill_Thrust : public UCSkill
{
	GENERATED_BODY()

public:
    virtual void Pressed() override;
    virtual void Begin_Skill_Implementation() override;
    virtual void End_Skill_Implementation() override;
    virtual void Tick_Implementation(float InDeltaTime) override;

private:
    UFUNCTION()
        void OnAttachmentBeginOverlap(
            ACCommonCharacter* InAttacker,
            AActor* InAttackCauser,
            ACCommonCharacter* InOther);

private:
    UPROPERTY(EditDefaultsOnly, Category = "Trace")
        float Distance = 1000;

    UPROPERTY(EditDefaultsOnly, Category = "Trace")
        float Speed = 200;

    UPROPERTY(EditDefaultsOnly, Category = "Trace")
        TEnumAsByte<EDrawDebugTrace::Type> DrawDebug;


    UPROPERTY(EditDefaultsOnly, Category = "Action")
        FActData ActData;

    UPROPERTY(EditDefaultsOnly, Category = "Action")
        FHitData HitData;

private:
    bool bMoving;
    FVector Start;
    FVector End;

    TArray<TWeakObjectPtr<ACCommonCharacter>> Overlapped;
    TArray<TWeakObjectPtr<ACCommonCharacter>> Hitted;
};
