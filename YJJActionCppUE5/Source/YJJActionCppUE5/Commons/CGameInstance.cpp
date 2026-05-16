#include "Commons/CGameInstance.h"
#include "Global.h"
#include "Internationalization/StringTable.h"

UCGameInstance::UCGameInstance()
{
	YJJHelpers::GetAsset(&CharacterInfoTable, "/Script/Engine.DataTable'/Game/Character/CDT_CharacterInfo.CDT_CharacterInfo'");
	YJJHelpers::GetAsset(&CharacterStatTable, "/Script/Engine.DataTable'/Game/Character/CDT_CharacterStat.CDT_CharacterStat'");

	YJJHelpers::GetAsset(&UiStringTable, "/Script/Engine.StringTable'/Game/Localization/ST_YJJ_UI.ST_YJJ_UI'");
	if (false == IsValid(UiStringTable))
	{
		CLog::Log(FString(TEXT("[Loc] ST_YJJ_UI String Table 없음 — Content/Localization/ST_YJJ_UI 생성 후 Source/YJJActionCppUE5/LocalizedText.csv Reimport")));
	}

	if (IsValid(CharacterInfoTable) && CharacterInfoTable->GetRowMap().Num() <= 0)
		CLog::Log("CharacterInfoTable is Empty.");
	if (IsValid(CharacterStatTable) && CharacterStatTable->GetRowMap().Num() <= 0)
		CLog::Log("CharacterStatTable is Empty.");
}

void UCGameInstance::Init()
{
	Super::Init();
}

FCCharacterInfo* UCGameInstance::GetInfo(const int32 InCharacterType) const
{
	return YJJHelpers::FindRow<FCCharacterInfo>(CharacterInfoTable, InCharacterType);
}

FCCharacterStat* UCGameInstance::GetStat(const int32 InLevel) const
{
	return YJJHelpers::FindRow<FCCharacterStat>(CharacterStatTable, InLevel);
}
