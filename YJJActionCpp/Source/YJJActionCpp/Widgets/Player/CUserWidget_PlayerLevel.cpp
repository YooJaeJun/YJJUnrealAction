#include "Widgets/Player/CUserWidget_PlayerLevel.h"
#include "Global.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UCUserWidget_PlayerLevel::BindLevelStat(UCCharacterStatComponent* InNewStat)
{
	CheckNull(InNewStat);

	CurStat = InNewStat;

	InNewStat->OnLevelChanged.AddUObject(this, &UCUserWidget_PlayerLevel::UpdateLevelWidget);
	InNewStat->OnExpChanged.AddUObject(this, &UCUserWidget_PlayerLevel::UpdateExpWidget);

	ProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("CProgressBar")));
	CurLevel = Cast<UTextBlock>(GetWidgetFromName(TEXT("CCurLevel")));

	UpdateLevelWidget();
	UpdateExpWidget();
}

void UCUserWidget_PlayerLevel::UpdateLevelWidget()
{
	CheckFalse(CurStat.IsValid());
	CheckNull(ProgressBar);
	CheckNull(CurLevel);

	ProgressBar->SetPercent(CurStat->GetExpRatio());
	CurLevel->SetText(FText::FromString(FString::FromInt(static_cast<int32>(CurStat->GetCurLevel()))));

	PlayLevelUpAnim();
}

void UCUserWidget_PlayerLevel::UpdateExpWidget()
{
	CheckFalse(CurStat.IsValid());
	CheckNull(ProgressBar);

	ProgressBar->SetPercent(CurStat->GetExpRatio());
}
