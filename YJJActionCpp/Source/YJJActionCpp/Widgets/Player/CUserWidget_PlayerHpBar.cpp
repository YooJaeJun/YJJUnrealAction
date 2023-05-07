#include "Widgets/Player/CUserWidget_PlayerHpBar.h"
#include "Global.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UCUserWidget_PlayerHpBar::BindCharacterStat(UCCharacterStatComponent* InNewStat)
{
	CheckNull(InNewStat);

	CurStat = InNewStat;

	InNewStat->OnHpChanged.AddUObject(this, &UCUserWidget_PlayerHpBar::UpdateHpWidget);

	HpProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("CHpProgressBar")));
	CurHp = Cast<UTextBlock>(GetWidgetFromName(TEXT("CCurHp")));
	MaxHp = Cast<UTextBlock>(GetWidgetFromName(TEXT("CMaxHp")));

	UpdateHpWidget();
}

void UCUserWidget_PlayerHpBar::UpdateHpWidget()
{
	CheckFalse(CurStat.IsValid());
	CheckNull(HpProgressBar);
	CheckNull(CurHp);
	CheckNull(MaxHp);

	HpProgressBar->SetPercent(CurStat->GetHpRatio());
	CurHp->SetText(FText::FromString(FString::FromInt(static_cast<int32>(CurStat->GetCurHp()))));
	MaxHp->SetText(FText::FromString(FString::FromInt(static_cast<int32>(CurStat->GetMaxHp()))));
}


