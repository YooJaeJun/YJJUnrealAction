#include "Widgets/CUserWidget_HUD.h"
#include "Global.h"
#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "Widgets/Weapons/CUserWidget_EquipMenu.h"
#include "Widgets/Weapons/CUserWidget_MagicMenu.h"
#include "Widgets/Interaction/CUserWidget_Interaction.h"

void UCUserWidget_HUD::SetChildren()
{
	if (nullptr == PlayerInfo)
	{
		PlayerInfo = Cast<UCUserWidget_PlayerInfo>(GetWidgetFromName(TEXT("CPlayerInfo")));
		if (IsValid(PlayerInfo))
			PlayerInfo->BindChildren();
	}

	if (nullptr == EquipMenu)
	{
		EquipMenu = Cast<UCUserWidget_EquipMenu>(GetWidgetFromName(TEXT("CEquipMenu")));
		if (IsValid(EquipMenu))
			EquipMenu->BindChildren();
	}

	if (nullptr == MagicMenu)
	{
		MagicMenu = Cast<UCUserWidget_MagicMenu>(GetWidgetFromName(TEXT("CMagicMenu")));
		if (IsValid(MagicMenu))
			MagicMenu->BindChildren();
	}

	if (nullptr == Interaction)
	{
		Interaction = Cast<UCUserWidget_Interaction>(GetWidgetFromName(TEXT("CInteraction")));
		if (IsValid(Interaction))
			Interaction->BindChildren();
	}
}