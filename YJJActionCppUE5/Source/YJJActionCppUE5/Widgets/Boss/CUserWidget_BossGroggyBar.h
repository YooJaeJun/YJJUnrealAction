#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_BossGroggyBar.generated.h"

class UTextBlock;
class UCUserWidget_BossGroggyBarGauge;

// 레거시 /Game/Widgets/Boss/WB_Boss_GroggyBar — Cur/Max 텍스트 + 자식 게이지 퍼센트 갱신.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API UCUserWidget_BossGroggyBar : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	static double ComputeGroggyRatio(const double InCur, const double InMax);

	UFUNCTION(BlueprintCallable, Category = "Boss|Groggy", meta = (DisplayName = "Set Groggy UI"))
	void SetGroggyUI(double InCur, double InMax);

	// CDragon 폴백 ProcessEvent 경로 — 게이지만 갱신.
	UFUNCTION(BlueprintCallable, Category = "Boss|Groggy", meta = (DisplayName = "Set Groggy Percent"))
	void SetGroggyPercent(double InGroggyPercent);

private:
	void EnsureWidgetBindings();
	UCUserWidget_BossGroggyBarGauge* ResolveGaugeWidget();

private:
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CurGroggyText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> MaxGroggyText;

	UPROPERTY(Transient)
	TObjectPtr<UCUserWidget_BossGroggyBarGauge> GaugeWidget;
};
