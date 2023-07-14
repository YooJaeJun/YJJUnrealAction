#pragma once
#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_EnemyBar.generated.h"

class UCCharacterStatComponent;
class UProgressBar;

UCLASS()
class YJJACTIONCPP_API UCUserWidget_EnemyBar : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindHpStat(UCCharacterStatComponent* InNewStat);

protected:
	void UpdateHpBarWidget() const;

private:
	UPROPERTY()
		UProgressBar* ProgressBar;

private:
	TWeakObjectPtr<UCCharacterStatComponent> CurStat;
};
