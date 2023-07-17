#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_PlayerLevel.generated.h"

class UCCharacterStatComponent;
class UProgressBar;
class UTextBlock;

UCLASS()
class YJJACTIONCPPUE5_API UCUserWidget_PlayerLevel : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindLevelStat(TObjectPtr<UCCharacterStatComponent> InNewStat);

protected:
	UFUNCTION(BlueprintImplementableEvent)
		void PlayLevelUpAnim();

	void UpdateLevelWidget();

	void UpdateExpWidget();

public:
	UPROPERTY()
		TObjectPtr<UProgressBar> ProgressBar;

	UPROPERTY()
		TObjectPtr<UTextBlock> CurLevel;

private:
	TWeakObjectPtr<UCCharacterStatComponent> CurStat;
};
