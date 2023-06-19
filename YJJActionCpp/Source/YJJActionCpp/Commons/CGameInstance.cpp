#include "Commons/CGameInstance.h"
#include "Global.h"

UCGameInstance::UCGameInstance()
	: UGameInstance()
{
	CHelpers::GetAsset(&CharacterInfoTable, "DataTable'/Game/Character/DT_CCharacterInfo.DT_CCharacterInfo'");
	CHelpers::GetAsset(&CharacterStatTable, "DataTable'/Game/Character/DT_CCharacterStat.DT_CCharacterStat'");

	if (CharacterInfoTable->GetRowMap().Num() <= 0)
		CLog::Log("CharacterInfoTable is Empty.");
	if (CharacterStatTable->GetRowMap().Num() <= 0)
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
