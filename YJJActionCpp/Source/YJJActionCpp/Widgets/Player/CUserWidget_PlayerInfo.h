#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_PlayerInfo.generated.h"

class UCUserWidget_PlayerBar;
class UCUserWidget_PlayerLevel;
class UCCharacterStatComponent;

UCLASS()
class YJJACTIONCPP_API UCUserWidget_PlayerInfo : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindChildren();
	void BindStats(UCCharacterStatComponent* StatComp);

public:
	UPROPERTY()
		UCUserWidget_PlayerLevel* LevelBar;

	UPROPERTY()
		UCUserWidget_PlayerBar* HpBar;

	UPROPERTY()
		UCUserWidget_PlayerBar* StaminaBar;

	UPROPERTY()
		UCUserWidget_PlayerBar* ManaBar;
};
