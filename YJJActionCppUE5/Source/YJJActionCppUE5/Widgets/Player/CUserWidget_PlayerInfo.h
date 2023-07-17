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

public:
	UPROPERTY()
		TObjectPtr<UCUserWidget_PlayerLevel> LevelBar;

	UPROPERTY()
		TObjectPtr<UCUserWidget_PlayerBar> HpBar;

	UPROPERTY()
		TObjectPtr<UCUserWidget_PlayerBar> StaminaBar;

	UPROPERTY()
		TObjectPtr<UCUserWidget_PlayerBar> ManaBar;
};
