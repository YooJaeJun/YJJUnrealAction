#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_PlayerBar.generated.h"

class UCCharacterStatComponent;
class UProgressBar;
class UTextBlock;

UCLASS()
class YJJACTIONCPPUE5_API UCUserWidget_PlayerBar : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindHpStat(TObjectPtr<UCCharacterStatComponent> InNewStat);
	void BindStaminaStat(TObjectPtr<UCCharacterStatComponent> InNewStat);
	void BindManaStat(TObjectPtr<UCCharacterStatComponent> InNewStat);

protected:
	void UpdateHpBarWidget();
	void UpdateStaminaBarWidget();
	void UpdateManaBarWidget();

private:
	UPROPERTY()
		TObjectPtr<UProgressBar> ProgressBar;

	UPROPERTY()
		TObjectPtr<UTextBlock> CurAmount;

	UPROPERTY()
		TObjectPtr<UTextBlock> MaxAmount;

private:
	TWeakObjectPtr<UCCharacterStatComponent> CurStat;
};
