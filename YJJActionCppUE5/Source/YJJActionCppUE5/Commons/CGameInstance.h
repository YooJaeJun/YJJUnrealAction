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
	UPROPERTY(EditAnywhere)
		int32 Type;

	UPROPERTY(EditAnywhere)
		int32 Group;

	UPROPERTY(EditAnywhere)
		FName Name;

	UPROPERTY(EditAnywhere)
		FLinearColor BodyColor;
};

USTRUCT(BlueprintType)
struct FCCharacterStat : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		int32 Level = 1;

	UPROPERTY(EditAnywhere)
		float MaxExp = 100;

	UPROPERTY(EditAnywhere)
		float MaxHp = 100;

	UPROPERTY(EditAnywhere)
		float MaxStamina = 100;

	UPROPERTY(EditAnywhere)
		float MaxMana = 100;

	UPROPERTY(EditAnywhere)
		float StaminaAccelRestore;

	UPROPERTY(EditAnywhere)
		float StaminaDefaultRestore;

	UPROPERTY(EditAnywhere)
		float ManaAccelRestore;

	UPROPERTY(EditAnywhere)
		float ManaDefaultRestore;

	UPROPERTY(EditAnywhere)
		float Attack;

	UPROPERTY(EditAnywhere)
		float DropExp;
};

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
};
