#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_PlayerInfo.generated.h"

class UCUserWidget_PlayerBar;
class UCUserWidget_PlayerLevel;
class UCCharacterStatComponent;

UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API UCUserWidget_PlayerInfo : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindChildren();
	void BindStats(TObjectPtr<UCCharacterStatComponent> StatComp);

	void RefreshPlayerInfoWidgets();

public:
	UPROPERTY(meta = (DisplayName = "LevelBar"))
	TObjectPtr<UCUserWidget_PlayerLevel> BoundLevelBar;

	UPROPERTY(meta = (DisplayName = "HpBar"))
	TObjectPtr<UCUserWidget_PlayerBar> BoundHpBar;

	UPROPERTY(meta = (DisplayName = "StaminaBar"))
	TObjectPtr<UCUserWidget_PlayerBar> BoundStaminaBar;

	UPROPERTY(meta = (DisplayName = "ManaBar"))
	TObjectPtr<UCUserWidget_PlayerBar> BoundManaBar;

private:
	// 동일 스탯에 대해 BindStats 가 반복 호출될 때 델리게이트가 중복 구독되지 않도록 막는다.
	TWeakObjectPtr<UCCharacterStatComponent> BoundStatComp;
};
