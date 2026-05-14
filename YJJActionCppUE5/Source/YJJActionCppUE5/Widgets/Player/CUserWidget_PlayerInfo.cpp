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

void UCUserWidget_PlayerInfo::BindStats(TObjectPtr<UCCharacterStatComponent> StatComp)
{
	if (false == IsValid(StatComp))
		return;

	if (BoundStatComp.Get() == StatComp)
		return;

	BoundStatComp = StatComp;

	if (IsValid(LevelBar))
		LevelBar->BindLevelStat(StatComp);

	if (IsValid(HpBar))
		HpBar->BindHpStat(StatComp);

	if (IsValid(StaminaBar))
		StaminaBar->BindStaminaStat(StatComp);

	if (IsValid(ManaBar))
		ManaBar->BindManaStat(StatComp);
}

void UCUserWidget_PlayerInfo::RefreshPlayerInfoWidgets()
{
	if (IsValid(HpBar))
		HpBar->RefreshBoundDisplay();

	if (IsValid(StaminaBar))
		StaminaBar->RefreshBoundDisplay();

	if (IsValid(ManaBar))
		ManaBar->RefreshBoundDisplay();

	if (IsValid(LevelBar))
		LevelBar->RefreshBoundDisplay();
}