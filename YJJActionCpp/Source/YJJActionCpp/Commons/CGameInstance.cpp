#include "Commons/CGameInstance.h"
#include "Global.h"

UCGameInstance::UCGameInstance()
{
	YJJHelpers::GetAsset(&CharacterInfoTable, "DataTable'/Game/Character/CDT_CharacterInfo.CDT_CharacterInfo'");
	YJJHelpers::GetAsset(&CharacterStatTable, "DataTable'/Game/Character/CDT_CharacterStat.CDT_CharacterStat'");

	if (!!CharacterInfoTable && CharacterInfoTable->GetRowMap().Num() <= 0)
		CLog::Log("CharacterInfoTable is Empty.");
	if (!!CharacterStatTable && CharacterStatTable->GetRowMap().Num() <= 0)
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
