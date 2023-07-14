#include "Widgets/Enemies/CUserWidget_EnemyBar.h"
#include "Global.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/ProgressBar.h"
#include "Widgets/Player/CUserWidget_PlayerBar.h"

void UCUserWidget_EnemyBar::BindHpStat(UCCharacterStatComponent* InNewStat)
{
	CheckNull(InNewStat);

	CurStat = InNewStat;

	InNewStat->OnHpChanged.AddUObject(this, &UCUserWidget_EnemyBar::UpdateHpBarWidget);

	ProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("CProgressBar")));

	UpdateHpBarWidget();
}

void UCUserWidget_EnemyBar::UpdateHpBarWidget() const
{
	CheckFalse(CurStat.IsValid());
	CheckNull(ProgressBar);

	ProgressBar->SetPercent(CurStat->GetHpRatio());
}