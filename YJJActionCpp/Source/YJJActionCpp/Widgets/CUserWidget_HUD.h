#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_HUD.generated.h"

class UCUserWidget_PlayerInfo;
class UCUserWidget_EquipMenu;
class UCUserWidget_Interaction;

UCLASS()
class YJJACTIONCPP_API UCUserWidget_HUD : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void SetChildren();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		UCUserWidget_PlayerInfo* PlayerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
		UCUserWidget_EquipMenu* EquipMenu;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
		UCUserWidget_Interaction* Interaction;
};
