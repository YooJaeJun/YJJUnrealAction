#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "CGameInstance.generated.h"

// 레거시 BP 사용자 정의 구조체 "FCharacter Info"(Group Index, Index, Body Color, Name)와 같은 페이로드.
// C++/데이터테이블 키는 기존대로 Type · Group 이름을 유지하고, 디스플레이 이름만 BP 필드명에 맞춘다.
USTRUCT(BlueprintType)
struct FCCharacterInfo : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (DisplayName = "Index"))
	int32 Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (DisplayName = "Group Index"))
	int32 Group;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FLinearColor BodyColor;
};

USTRUCT(BlueprintType)
struct FCCharacterStat : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxExp = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxHp = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxStamina = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxMana = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float StaminaAccelRestore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float StaminaDefaultRestore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float ManaAccelRestore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float ManaDefaultRestore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float DropExp;
};

class UStringTable;

UCLASS()
class YJJACTIONCPPUE5_API UCGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UCGameInstance();

	virtual void Init() override;

	FCCharacterInfo* GetInfo(const int32 InCharacterType) const;
	FCCharacterStat* GetStat(const int32 InLevel) const;

private:
	UPROPERTY()
	TObjectPtr<UDataTable> CharacterInfoTable;

	UPROPERTY()
	TObjectPtr<UDataTable> CharacterStatTable;

	// YJJLocalization::LocalizedText_From_UserInterface_Localization_Table_Key 가 같은 Identifier 사용 — Content/Localization/ST_YJJ_UI; 원본 CSV 는 Source/YJJActionCppUE5/LocalizedText.csv .
	UPROPERTY()
	TObjectPtr<UStringTable> UiStringTable;
};
