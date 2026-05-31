#include "Commons/CGameInstance.h"
#include "Global.h"
#include "Internationalization/StringTable.h"

UCGameInstance::UCGameInstance()
{
	// 캐릭터 데이터 테이블은 CDT_* 가 아니라 DT_* 네이밍 — Content 에셟명과 경로 동기화 필요.
	YJJHelpers::GetAsset(&CharacterInfoTable, "/Script/Engine.DataTable'/Game/Character/DT_CharacterInfo.DT_CharacterInfo'");
	YJJHelpers::GetAsset(&CharacterStatTable, "/Script/Engine.DataTable'/Game/Character/DT_CharacterStat.DT_CharacterStat'");

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
