#pragma once

#include "CoreMinimal.h"
#include "Widgets/Enemies/CUserWidget_EnemyBar.h"
#include "CUserWidget_BossGroggyBarGauge.generated.h"

class UProgressBar;

// 레거시 /Game/Widgets/Boss/WB_Boss_GroggyBar_Guage — CUserWidget_EnemyBar 기반 그로기 게이지.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API UCUserWidget_BossGroggyBarGauge : public UCUserWidget_EnemyBar
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Groggy", meta = (DisplayName = "Set Groggy Percent"))
	void SetGroggyPercent(double InGroggyPercent);

private:
	UProgressBar* ResolveGroggyProgressBar();

private:
	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> GroggyProgressBar;
};
