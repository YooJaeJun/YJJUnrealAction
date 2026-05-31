#include "Widgets/CUserWidget_HUD.h"
#include "Global.h"
#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "Widgets/Weapons/CUserWidget_EquipMenu.h"
#include "Widgets/Weapons/CUserWidget_MagicMenu.h"
#include "Widgets/Interaction/CUserWidget_Interaction.h"

namespace
{
	UWidget* FindChildWidgetByNames(UUserWidget* Root, const TArray<FName>& CandidateNames)
	{
		if (false == IsValid(Root))
			return nullptr;

		const int32 nameCount = CandidateNames.Num();
		for (int32 nameIndex = 0; nameIndex < nameCount; ++nameIndex)
		{
			UWidget* const found = Root->GetWidgetFromName(CandidateNames[nameIndex]);
			if (IsValid(found))
				return found;
		}

		return nullptr;
	}
}

void UCUserWidget_HUD::SetChildren()
{
	if (nullptr == BoundPlayerInfo)
	{
		static const FName PlayerInfoNames[] = {
			FName(TEXT("CPlayerInfo")),
			FName(TEXT("WB_PlayerInfo")),
			FName(TEXT("PlayerInfo")),
		};
		BoundPlayerInfo = Cast<UCUserWidget_PlayerInfo>(
			FindChildWidgetByNames(this, TArray<FName>(PlayerInfoNames, UE_ARRAY_COUNT(PlayerInfoNames))));
		if (IsValid(BoundPlayerInfo))
			BoundPlayerInfo->BindChildren();
	}

	if (nullptr == BoundEquipMenu)
	{
		static const FName EquipMenuNames[] = {
			FName(TEXT("CEquipMenu")),
			FName(TEXT("WB_EquipMenu")),
			FName(TEXT("EquipMenu")),
		};
		BoundEquipMenu = Cast<UCUserWidget_EquipMenu>(
			FindChildWidgetByNames(this, TArray<FName>(EquipMenuNames, UE_ARRAY_COUNT(EquipMenuNames))));
		if (IsValid(BoundEquipMenu))
			BoundEquipMenu->BindChildren();
	}

	if (nullptr == BoundMagicMenu)
	{
		static const FName MagicMenuNames[] = {
			FName(TEXT("CMagicMenu")),
			FName(TEXT("WB_MagicMenu")),
			FName(TEXT("MagicMenu")),
		};
		BoundMagicMenu = Cast<UCUserWidget_MagicMenu>(
			FindChildWidgetByNames(this, TArray<FName>(MagicMenuNames, UE_ARRAY_COUNT(MagicMenuNames))));
		if (IsValid(BoundMagicMenu))
			BoundMagicMenu->BindChildren();
	}

	if (nullptr == BoundInteraction)
	{
		static const FName InteractionNames[] = {
			FName(TEXT("CInteraction")),
			FName(TEXT("WB_Interaction")),
			FName(TEXT("Interaction")),
		};
		BoundInteraction = Cast<UCUserWidget_Interaction>(
			FindChildWidgetByNames(this, TArray<FName>(InteractionNames, UE_ARRAY_COUNT(InteractionNames))));
		if (IsValid(BoundInteraction))
			BoundInteraction->BindChildren();
	}

	if (false == IsValid(HudMessageSlotWidget))
		HudMessageSlotWidget = Cast<UUserWidget>(GetWidgetFromName(TEXT("WB_Message")));
	if (false == IsValid(HudMessageSlotWidget))
		HudMessageSlotWidget = Cast<UUserWidget>(GetWidgetFromName(TEXT("CMessage")));
}

UUserWidget* UCUserWidget_HUD::ResolveWB_MessageWidget()
{
	if (false == IsValid(HudMessageSlotWidget))
		HudMessageSlotWidget = Cast<UUserWidget>(GetWidgetFromName(TEXT("WB_Message")));
	if (false == IsValid(HudMessageSlotWidget))
		HudMessageSlotWidget = Cast<UUserWidget>(GetWidgetFromName(TEXT("CMessage")));
	return HudMessageSlotWidget;
}