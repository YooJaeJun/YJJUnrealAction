#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "CUserWidget_PlayerBar.h"
#include "CUserWidget_PlayerLevel.h"
#include "Components/CCharacterStatComponent.h"

void UCUserWidget_PlayerInfo::BindChildren()
{
	LevelBar = Cast<UCUserWidget_PlayerLevel>(GetWidgetFromName(TEXT("CLevelBar")));
	HpBar = Cast<UCUserWidget_PlayerBar>(GetWidgetFromName(TEXT("CHpBar")));
	StaminaBar = Cast<UCUserWidget_PlayerBar>(GetWidgetFromName(TEXT("CStaminaBar")));
	ManaBar = Cast<UCUserWidget_PlayerBar>(GetWidgetFromName(TEXT("CManaBar")));
}

void UCUserWidget_PlayerInfo::BindStats(UCCharacterStatComponent* StatComp)
{
	if (IsValid(LevelBar))
		LevelBar->BindLevelStat(StatComp);

	if (IsValid(HpBar))
		HpBar->BindHpStat(StatComp);

	if (IsValid(StaminaBar))
		StaminaBar->BindStaminaStat(StatComp);

	if (IsValid(ManaBar))
		ManaBar->BindManaStat(StatComp);
}