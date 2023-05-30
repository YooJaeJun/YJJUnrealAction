#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_PlayerInfo.generated.h"

class UCUserWidget_PlayerBar;
class UCUserWidget_PlayerLevel;

UCLASS()
class YJJACTIONCPP_API UCUserWidget_PlayerInfo : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindChildren();

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
