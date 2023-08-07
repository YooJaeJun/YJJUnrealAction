#pragma once
#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_EnemyInfo.generated.h"

class UCCharacterInfoComponent;
class UCCharacterStatComponent;
class UTextBlock;
class UProgressBar;

UCLASS()
class YJJACTIONCPP_API UCUserWidget_EnemyInfo : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindChildren(UCCharacterInfoComponent* InNewInfo, UCCharacterStatComponent* InNewStat);
	void UpdateEnemyBarWidget() const;

private:
	UPROPERTY()
		UTextBlock* CharacterName;

	UPROPERTY()
		UProgressBar* ProgressBar;

private:
	TWeakObjectPtr<UCCharacterInfoComponent> CurInfo;
	TWeakObjectPtr<UCCharacterStatComponent> CurStat;
};
