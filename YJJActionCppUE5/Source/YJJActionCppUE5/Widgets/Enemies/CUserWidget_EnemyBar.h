#pragma once
#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_EnemyBar.generated.h"

class UCCharacterStatComponent;
class UProgressBar;

UCLASS()
class YJJACTIONCPPUE5_API UCUserWidget_EnemyBar : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindHpStat(TObjectPtr<UCCharacterStatComponent> InNewStat);

protected:
	void UpdateHpBarWidget() const;

private:
	UPROPERTY()
		TObjectPtr<UProgressBar> ProgressBar;

private:
	TWeakObjectPtr<UCCharacterStatComponent> CurStat;
};
