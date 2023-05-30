#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "CUserWidget_PlayerBar.h"
#include "CUserWidget_PlayerLevel.h"

void UCUserWidget_PlayerInfo::BindChildren()
{
	LevelBar = Cast<UCUserWidget_PlayerLevel>(GetWidgetFromName(TEXT("CLevelBar")));
	HpBar = Cast<UCUserWidget_PlayerBar>(GetWidgetFromName(TEXT("CHpBar")));
	StaminaBar = Cast<UCUserWidget_PlayerBar>(GetWidgetFromName(TEXT("CStaminaBar")));
	ManaBar = Cast<UCUserWidget_PlayerBar>(GetWidgetFromName(TEXT("CManaBar")));
}
