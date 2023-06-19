#include "Commons/CGameInstance.h"
#include "Global.h"

UCGameInstance::UCGameInstance()
{
	CHelpers::GetAsset(&CharacterInfoTable, "DataTable'/Game/Character/CDT_CharacterInfo.CDT_CharacterInfo'");
	CHelpers::GetAsset(&CharacterStatTable, "DataTable'/Game/Character/CDT_CharacterStat.CDT_CharacterStat'");

	if (!!CharacterInfoTable && CharacterInfoTable->GetRowMap().Num() <= 0)
		CLog::Log("CharacterInfoTable is Empty.");
	if (!!CharacterStatTable && CharacterStatTable->GetRowMap().Num() <= 0)
		CLog::Log("CharacterStatTable is Empty.");
}

void UCGameInstance::Init()
{
	Super::Init();
}

FCCharacterInfo* UCGameInstance::GetInfo(const int32 InGroupIndex) const
{
	return CharacterInfoTable->FindRow<FCCharacterInfo>(*FString::FromInt(InGroupIndex), TEXT(""));
}

FCCharacterStat* UCGameInstance::GetStat(const int32 InLevel) const
{
	return CharacterStatTable->FindRow<FCCharacterStat>(*FString::FromInt(InLevel), TEXT(""));
}
