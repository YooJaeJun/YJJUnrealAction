#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "CGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FCCharacterInfo : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Status")
		FName Name;

	UPROPERTY(EditAnywhere, Category = "Status")
		int32 GroupIndex;

	UPROPERTY(EditAnywhere, Category = "Status")
		FLinearColor BodyColor;
};

USTRUCT(BlueprintType)
struct FCCharacterStat : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Status")
		int32 Level = 1;

	UPROPERTY(EditAnywhere, Category = "Status")
		float MaxExp = 100;

	UPROPERTY(EditAnywhere, Category = "Status")
		float MaxHp = 100;

	UPROPERTY(EditAnywhere, Category = "Status")
		float MaxStamina = 100;

	UPROPERTY(EditAnywhere, Category = "Status")
		float MaxMana = 100;

	UPROPERTY(EditAnywhere, Category = "Status")
		float StaminaAccelRestore;

	UPROPERTY(EditAnywhere, Category = "Status")
		float StaminaDefaultRestore;

	UPROPERTY(EditAnywhere, Category = "Status")
		float ManaAccelRestore;

	UPROPERTY(EditAnywhere, Category = "Status")
		float ManaDefaultRestore;

	UPROPERTY(EditAnywhere, Category = "Status")
		float Attack;

	UPROPERTY(EditAnywhere, Category = "Status")
		float DropExp;
};

UCLASS()
class YJJACTIONCPP_API UCGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UCGameInstance();

	void Init() override;

	FCCharacterInfo* GetInfo(const FName InName);
	FCCharacterStat* GetStat(int32 InLevel);

private:
	UPROPERTY()
		UDataTable* CharacterInfoTable;

	UPROPERTY()
		UDataTable* CharacterStatTable;
};
