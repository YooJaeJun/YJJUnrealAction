#include "Widgets/Boss/CUserWidget_BossGroggyBarGauge.h"
#include "Global.h"
#include "Components/ProgressBar.h"

UProgressBar* UCUserWidget_BossGroggyBarGauge::ResolveGroggyProgressBar()
{
	if (IsValid(GroggyProgressBar))
	{
		return GroggyProgressBar;
	}

	GroggyProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("GroggyBar")));
	if (IsValid(GroggyProgressBar))
	{
		return GroggyProgressBar;
	}

	GroggyProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("CProgressBar")));
	return GroggyProgressBar;
}

void UCUserWidget_BossGroggyBarGauge::SetGroggyPercent(const double InGroggyPercent)
{
	UProgressBar* progressBar = ResolveGroggyProgressBar();
	if (false == IsValid(progressBar))
	{
		CLog::Log(TEXT("[UCUserWidget_BossGroggyBarGauge::SetGroggyPercent] GroggyBar ProgressBar 위젯을 찾지 못함"));
		return;
	}

	const float clampedPercent = FMath::Clamp(static_cast<float>(InGroggyPercent), 0.0f, 1.0f);
	progressBar->SetPercent(clampedPercent);
}
