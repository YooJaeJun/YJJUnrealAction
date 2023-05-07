#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_PlayerBar.generated.h"

class UCCharacterStatComponent;
class UProgressBar;
class UTextBlock;

UCLASS()
class YJJACTIONCPP_API UCUserWidget_PlayerBar : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindHpStat(UCCharacterStatComponent* InNewStat);
	void BindStaminaStat(UCCharacterStatComponent* InNewStat);
	void BindManaStat(UCCharacterStatComponent* InNewStat);

protected:
	void UpdateHpBarWidget();
	void UpdateStaminaBarWidget();
	void UpdateManaBarWidget();

private:
	UPROPERTY()
		UProgressBar* ProgressBar;

	UPROPERTY()
		UTextBlock* CurAmount;

	UPROPERTY()
		UTextBlock* MaxAmount;

private:
	TWeakObjectPtr<UCCharacterStatComponent> CurStat;
};
