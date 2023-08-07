#include "Widgets/Enemies/CUserWidget_EnemyInfo.h"
#include "Global.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Widgets/Player/CUserWidget_PlayerBar.h"

void UCUserWidget_EnemyInfo::BindChildren(UCCharacterInfoComponent* InNewInfo, UCCharacterStatComponent* InNewStat)
{
	CheckNull(InNewInfo);
	CheckNull(InNewStat);

	CurInfo = InNewInfo;
	CurStat = InNewStat;

	InNewStat->OnHpChanged.AddUObject(this, &UCUserWidget_EnemyInfo::UpdateEnemyBarWidget);

	CharacterName = Cast<UTextBlock>(GetWidgetFromName(TEXT("CName")));
	ProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("CProgressBar")));

	UpdateEnemyBarWidget();
}

void UCUserWidget_EnemyInfo::UpdateEnemyBarWidget() const
{
	CheckFalse(CurStat.IsValid());
	CheckNull(CharacterName);
	CheckNull(ProgressBar);

	CharacterName->SetText(FText::FromName(CurInfo->GetCharacterName()));
	ProgressBar->SetPercent(CurStat->GetHpRatio());
}