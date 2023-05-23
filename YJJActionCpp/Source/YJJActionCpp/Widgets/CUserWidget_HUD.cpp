#include "Widgets/CUserWidget_HUD.h"
#include "Global.h"
#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "Widgets/Weapons/CUserWidget_EquipMenu.h"
#include "Widgets/Interaction/CUserWidget_Interaction.h"

void UCUserWidget_HUD::SetChild()
{
	if (nullptr == PlayerInfo)
	{
		PlayerInfo = Cast<UCUserWidget_PlayerInfo>(GetWidgetFromName(TEXT("CPlayerInfo")));
		PlayerInfo->SetChild();
	}

	if (nullptr == EquipMenu)
	{
		EquipMenu = Cast<UCUserWidget_EquipMenu>(GetWidgetFromName(TEXT("CEquipMenu")));
		EquipMenu->SetChild();
	}

	if (nullptr == Interaction)
	{
		Interaction = Cast<UCUserWidget_Interaction>(GetWidgetFromName(TEXT("CInteraction")));
		Interaction->SetChild();
	}
}