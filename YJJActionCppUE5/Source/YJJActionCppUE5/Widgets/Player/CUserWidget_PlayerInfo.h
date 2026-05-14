#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_PlayerInfo.generated.h"

class UCUserWidget_PlayerBar;
class UCUserWidget_PlayerLevel;
class UCCharacterStatComponent;

UCLASS()
class YJJACTIONCPPUE5_API UCUserWidget_PlayerInfo : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindChildren();
	void BindStats(TObjectPtr<UCCharacterStatComponent> StatComp);

	void RefreshPlayerInfoWidgets();

public:
	UPROPERTY()
	TObjectPtr<UCUserWidget_PlayerLevel> LevelBar;

	UPROPERTY()
	TObjectPtr<UCUserWidget_PlayerBar> HpBar;

	UPROPERTY()
	TObjectPtr<UCUserWidget_PlayerBar> StaminaBar;

	UPROPERTY()
	TObjectPtr<UCUserWidget_PlayerBar> ManaBar;

private:
	// 동일 스탯에 대해 BindStats 가 반복 호출될 때 델리게이트가 중복 구독되지 않도록 막는다.
	TWeakObjectPtr<UCCharacterStatComponent> BoundStatComp;
};
