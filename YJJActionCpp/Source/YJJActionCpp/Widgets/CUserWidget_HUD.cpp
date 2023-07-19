#include "Widgets/CUserWidget_HUD.h"
#include "Global.h"
#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "Widgets/Weapons/CUserWidget_EquipMenu.h"
#include "Widgets/Interaction/CUserWidget_Interaction.h"
#include "Widgets/Weapons/CUserWidget_CrossHair.h"

void UCUserWidget_HUD::SetChildren()
{
	if (false == IsValid(PlayerInfo))
	{
		PlayerInfo = Cast<UCUserWidget_PlayerInfo>(GetWidgetFromName(TEXT("CPlayerInfo")));
		PlayerInfo->BindChildren();
	}

	if (false == IsValid(EquipMenu))
	{
		EquipMenu = Cast<UCUserWidget_EquipMenu>(GetWidgetFromName(TEXT("CEquipMenu")));
		EquipMenu->BindChildren();
	}

	if (false == IsValid(Interaction))
	{
		Interaction = Cast<UCUserWidget_Interaction>(GetWidgetFromName(TEXT("CInteraction")));
		Interaction->BindChildren();
	}

	if (false == IsValid(CrossHair))
		CrossHair = Cast<UCUserWidget_CrossHair>(GetWidgetFromName(TEXT("CCrossHair")));
}