#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_PlayerHpBar.generated.h"

class UCCharacterStatComponent;
class UProgressBar;
class UTextBlock;

UCLASS()
class YJJACTIONCPP_API UCUserWidget_PlayerHpBar : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindCharacterStat(UCCharacterStatComponent* InNewStat);

protected:
	virtual void NativeConstruct() override;
	void UpdateHpWidget();

private:
	UPROPERTY()
		UProgressBar* HpProgressBar;

	UPROPERTY()
		UTextBlock* CurHp;

	UPROPERTY()
		UTextBlock* MaxHp;

private:
	TWeakObjectPtr<UCCharacterStatComponent> CurStat;
};
