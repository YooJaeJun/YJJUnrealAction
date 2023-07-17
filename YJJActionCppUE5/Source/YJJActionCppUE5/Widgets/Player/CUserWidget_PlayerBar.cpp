#include "Widgets/Player/CUserWidget_PlayerBar.h"
#include "Global.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UCUserWidget_PlayerBar::BindHpStat(TObjectPtr<UCCharacterStatComponent> InNewStat)
{
	CheckNull(InNewStat);

	CurStat = InNewStat;

	InNewStat->OnHpChanged.AddUObject(this, &UCUserWidget_PlayerBar::UpdateHpBarWidget);

	ProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("CProgressBar")));
	CurAmount = Cast<UTextBlock>(GetWidgetFromName(TEXT("CCurAmount")));
	MaxAmount = Cast<UTextBlock>(GetWidgetFromName(TEXT("CMaxAmount")));

	UpdateHpBarWidget();
}

void UCUserWidget_PlayerBar::BindStaminaStat(TObjectPtr<UCCharacterStatComponent> InNewStat)
{
	CheckNull(InNewStat);

	CurStat = InNewStat;

	InNewStat->OnStaminaChanged.AddUObject(this, &UCUserWidget_PlayerBar::UpdateStaminaBarWidget);

	ProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("CProgressBar")));
	CurAmount = Cast<UTextBlock>(GetWidgetFromName(TEXT("CCurAmount")));
	MaxAmount = Cast<UTextBlock>(GetWidgetFromName(TEXT("CMaxAmount")));

	UpdateStaminaBarWidget();
}

void UCUserWidget_PlayerBar::BindManaStat(TObjectPtr<UCCharacterStatComponent> InNewStat)
{
	CheckNull(InNewStat);

	CurStat = InNewStat;

	InNewStat->OnManaChanged.AddUObject(this, &UCUserWidget_PlayerBar::UpdateManaBarWidget);

	ProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("CProgressBar")));
	CurAmount = Cast<UTextBlock>(GetWidgetFromName(TEXT("CCurAmount")));
	MaxAmount = Cast<UTextBlock>(GetWidgetFromName(TEXT("CMaxAmount")));

	UpdateManaBarWidget();
}

void UCUserWidget_PlayerBar::UpdateHpBarWidget()
{
	CheckFalse(CurStat.IsValid());
	CheckNull(ProgressBar);
	CheckNull(CurAmount);
	CheckNull(MaxAmount);

	ProgressBar->SetPercent(CurStat->GetHpRatio());
	CurAmount->SetText(FText::FromString(FString::FromInt(static_cast<int32>(CurStat->GetCurHp()))));
	MaxAmount->SetText(FText::FromString(FString::FromInt(static_cast<int32>(CurStat->GetMaxHp()))));
}

void UCUserWidget_PlayerBar::UpdateStaminaBarWidget()
{
	CheckFalse(CurStat.IsValid());
	CheckNull(ProgressBar);
	CheckNull(CurAmount);
	CheckNull(MaxAmount);

	ProgressBar->SetPercent(CurStat->GetStaminaRatio());
	CurAmount->SetText(FText::FromString(FString::FromInt(static_cast<int32>(CurStat->GetCurStamina()))));
	MaxAmount->SetText(FText::FromString(FString::FromInt(static_cast<int32>(CurStat->GetMaxStamina()))));
}

void UCUserWidget_PlayerBar::UpdateManaBarWidget()
{
	CheckFalse(CurStat.IsValid());
	CheckNull(ProgressBar);
	CheckNull(CurAmount);
	CheckNull(MaxAmount);

	ProgressBar->SetPercent(CurStat->GetManaRatio());
	CurAmount->SetText(FText::FromString(FString::FromInt(static_cast<int32>(CurStat->GetCurMana()))));
	MaxAmount->SetText(FText::FromString(FString::FromInt(static_cast<int32>(CurStat->GetMaxMana()))));
}