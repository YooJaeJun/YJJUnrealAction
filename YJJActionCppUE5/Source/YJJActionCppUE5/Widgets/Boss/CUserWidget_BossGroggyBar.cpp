#include "Widgets/Boss/CUserWidget_BossGroggyBar.h"
#include "Global.h"
#include "Components/TextBlock.h"
#include "Widgets/Boss/CUserWidget_BossGroggyBarGauge.h"

double UCUserWidget_BossGroggyBar::ComputeGroggyRatio(const double InCur, const double InMax)
{
	if (InMax <= static_cast<double>(KINDA_SMALL_NUMBER))
	{
		return 0.0;
	}

	return FMath::Clamp(InCur / InMax, 0.0, 1.0);
}

void UCUserWidget_BossGroggyBar::EnsureWidgetBindings()
{
	if (false == IsValid(CurGroggyText))
	{
		CurGroggyText = Cast<UTextBlock>(GetWidgetFromName(TEXT("CurGroggy")));
	}

	if (false == IsValid(MaxGroggyText))
	{
		MaxGroggyText = Cast<UTextBlock>(GetWidgetFromName(TEXT("MaxGroggy")));
	}

	if (false == IsValid(GaugeWidget))
	{
		GaugeWidget = Cast<UCUserWidget_BossGroggyBarGauge>(GetWidgetFromName(TEXT("WB_Boss_GroggyBar_Guage")));
	}
}

UCUserWidget_BossGroggyBarGauge* UCUserWidget_BossGroggyBar::ResolveGaugeWidget()
{
	EnsureWidgetBindings();

	if (false == IsValid(GaugeWidget))
	{
		CLog::Log(TEXT("[UCUserWidget_BossGroggyBar] WB_Boss_GroggyBar_Guage 자식 위젯을 찾지 못함"));
		return nullptr;
	}

	return GaugeWidget;
}

void UCUserWidget_BossGroggyBar::SetGroggyUI(const double InCur, const double InMax)
{
	EnsureWidgetBindings();

	if (IsValid(CurGroggyText))
	{
		const int32 curDisplay = FMath::FloorToInt(static_cast<float>(InCur));
		CurGroggyText->SetText(FText::AsNumber(curDisplay));
	}

	if (IsValid(MaxGroggyText))
	{
		const int32 maxDisplay = FMath::FloorToInt(static_cast<float>(InMax));
		MaxGroggyText->SetText(FText::AsNumber(maxDisplay));
	}

	SetGroggyPercent(ComputeGroggyRatio(InCur, InMax));
}

void UCUserWidget_BossGroggyBar::SetGroggyPercent(const double InGroggyPercent)
{
	UCUserWidget_BossGroggyBarGauge* gaugeWidget = ResolveGaugeWidget();
	if (nullptr == gaugeWidget)
	{
		return;
	}

	gaugeWidget->SetGroggyPercent(InGroggyPercent);
}
