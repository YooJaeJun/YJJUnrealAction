#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "Global.h"
#include "CUserWidget_PlayerHpBar.h"

void UCUserWidget_PlayerInfo::SetChild()
{
	HpBar = Cast<UCUserWidget_PlayerHpBar>(GetWidgetFromName(TEXT("CHpBar")));
}
