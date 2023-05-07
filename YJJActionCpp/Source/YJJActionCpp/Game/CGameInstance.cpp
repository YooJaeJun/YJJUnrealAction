#include "Game/CGameInstance.h"
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

FCCharacterInfo* UCGameInstance::GetInfo(const FName InName)
{
	return CharacterInfoTable->FindRow<FCCharacterInfo>(InName, TEXT(""));
}

FCCharacterStat* UCGameInstance::GetStat(int32 InLevel)
{
	return CharacterStatTable->FindRow<FCCharacterStat>(*FString::FromInt(InLevel), TEXT(""));
}
