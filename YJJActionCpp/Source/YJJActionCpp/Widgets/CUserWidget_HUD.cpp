#include "Widgets/CUserWidget_HUD.h"
#include "Global.h"
#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "Widgets/Weapons/CUserWidget_EquipMenu.h"
#include "Widgets/Interaction/CUserWidget_Interaction.h"

void UCUserWidget_HUD::SetChildren()
{
	if (IsValid(PlayerInfo))
	{
		PlayerInfo = Cast<UCUserWidget_PlayerInfo>(GetWidgetFromName(TEXT("CPlayerInfo")));
		PlayerInfo->BindChildren();
	}

	if (IsValid(EquipMenu))
	{
		EquipMenu = Cast<UCUserWidget_EquipMenu>(GetWidgetFromName(TEXT("CEquipMenu")));
		EquipMenu->BindChildren();
	}

	if (IsValid(Interaction))
	{
		Interaction = Cast<UCUserWidget_Interaction>(GetWidgetFromName(TEXT("CInteraction")));
		Interaction->BindChildren();
	}

	if (IsValid(CrossHair))
		CrossHair = Cast<UCUserWidget_Custom>(GetWidgetFromName(TEXT("CCrossHair")));
}