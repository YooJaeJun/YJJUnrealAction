#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_PlayerBar.generated.h"

class UCCharacterStatComponent;
class UProgressBar;
class UTextBlock;

UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API UCUserWidget_PlayerBar : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindHpStat(TObjectPtr<UCCharacterStatComponent> InNewStat);
	void BindStaminaStat(TObjectPtr<UCCharacterStatComponent> InNewStat);
	void BindManaStat(TObjectPtr<UCCharacterStatComponent> InNewStat);

	// Bind* 이후 현재 바인딩된 스탯 값으로 게이지/텍스트만 다시 반영한다(SetStatusUI 등).
	void RefreshBoundDisplay();

protected:
	void UpdateHpBarWidget();
	void UpdateStaminaBarWidget();
	void UpdateManaBarWidget();

private:
	enum class EBoundResource : uint8
	{
		None,
		Hp,
		Stamina,
		Mana,
	};

	EBoundResource BoundResource = EBoundResource::None;

	UPROPERTY()
	TObjectPtr<UProgressBar> ProgressBar;

	UPROPERTY()
	TObjectPtr<UTextBlock> CurAmount;

	UPROPERTY()
	TObjectPtr<UTextBlock> MaxAmount;

private:
	TWeakObjectPtr<UCCharacterStatComponent> CurStat;
};
