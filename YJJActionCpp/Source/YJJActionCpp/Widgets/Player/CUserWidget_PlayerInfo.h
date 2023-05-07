#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_PlayerInfo.generated.h"

class UCUserWidget_PlayerHpBar;

UCLASS()
class YJJACTIONCPP_API UCUserWidget_PlayerInfo : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void SetChild();

public:
	UPROPERTY()
		UCUserWidget_PlayerHpBar* HpBar;
};
