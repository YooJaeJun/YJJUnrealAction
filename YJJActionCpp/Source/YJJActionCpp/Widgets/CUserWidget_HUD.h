#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_HUD.generated.h"

class UCUserWidget_PlayerInfo;
class UCUserWidget_EquipMenu;

UCLASS()
class YJJACTIONCPP_API UCUserWidget_HUD : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void SetChild();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		UCUserWidget_PlayerInfo* PlayerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
		UCUserWidget_EquipMenu* EquipMenu;
};
