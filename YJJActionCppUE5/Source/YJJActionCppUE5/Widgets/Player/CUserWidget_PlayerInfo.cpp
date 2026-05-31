#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "CUserWidget_PlayerBar.h"
#include "CUserWidget_PlayerLevel.h"
#include "Components/CCharacterStatComponent.h"

namespace
{
	template<typename TWidget>
	TWidget* FindChildWidgetByNames(UUserWidget* Root, const TArray<FName>& CandidateNames)
	{
		if (false == IsValid(Root))
			return nullptr;

		const int32 nameCount = CandidateNames.Num();
		for (int32 nameIndex = 0; nameIndex < nameCount; ++nameIndex)
		{
			TWidget* const found = Cast<TWidget>(Root->GetWidgetFromName(CandidateNames[nameIndex]));
			if (IsValid(found))
				return found;
		}

		return nullptr;
	}
}

void UCUserWidget_PlayerInfo::BindChildren()
{
	static const FName LevelBarNames[] = {
		FName(TEXT("CLevelBar")),
		FName(TEXT("WB_Player_Level")),
		FName(TEXT("LevelBar")),
	};
	BoundLevelBar = FindChildWidgetByNames<UCUserWidget_PlayerLevel>(
		this, TArray<FName>(LevelBarNames, UE_ARRAY_COUNT(LevelBarNames)));

	static const FName HpBarNames[] = {
		FName(TEXT("CHpBar")),
		FName(TEXT("WB_Player_HpBar")),
		FName(TEXT("HpBar")),
	};
	BoundHpBar = FindChildWidgetByNames<UCUserWidget_PlayerBar>(
		this, TArray<FName>(HpBarNames, UE_ARRAY_COUNT(HpBarNames)));

	static const FName StaminaBarNames[] = {
		FName(TEXT("CStaminaBar")),
		FName(TEXT("WB_Player_StaminaBar")),
		FName(TEXT("StaminaBar")),
	};
	BoundStaminaBar = FindChildWidgetByNames<UCUserWidget_PlayerBar>(
		this, TArray<FName>(StaminaBarNames, UE_ARRAY_COUNT(StaminaBarNames)));

	static const FName ManaBarNames[] = {
		FName(TEXT("CManaBar")),
		FName(TEXT("WB_Player_ManaBar")),
		FName(TEXT("ManaBar")),
	};
	BoundManaBar = FindChildWidgetByNames<UCUserWidget_PlayerBar>(
		this, TArray<FName>(ManaBarNames, UE_ARRAY_COUNT(ManaBarNames)));
}

void UCUserWidget_PlayerInfo::BindStats(TObjectPtr<UCCharacterStatComponent> StatComp)
{
	if (false == IsValid(StatComp))
		return;

	if (BoundStatComp.Get() == StatComp)
		return;

	BoundStatComp = StatComp;

	if (IsValid(BoundLevelBar))
		BoundLevelBar->BindLevelStat(StatComp);

	if (IsValid(BoundHpBar))
		BoundHpBar->BindHpStat(StatComp);

	if (IsValid(BoundStaminaBar))
		BoundStaminaBar->BindStaminaStat(StatComp);

	if (IsValid(BoundManaBar))
		BoundManaBar->BindManaStat(StatComp);
}

void UCUserWidget_PlayerInfo::RefreshPlayerInfoWidgets()
{
	if (IsValid(BoundHpBar))
		BoundHpBar->RefreshBoundDisplay();

	if (IsValid(BoundStaminaBar))
		BoundStaminaBar->RefreshBoundDisplay();

	if (IsValid(BoundManaBar))
		BoundManaBar->RefreshBoundDisplay();

	if (IsValid(BoundLevelBar))
		BoundLevelBar->RefreshBoundDisplay();
}
