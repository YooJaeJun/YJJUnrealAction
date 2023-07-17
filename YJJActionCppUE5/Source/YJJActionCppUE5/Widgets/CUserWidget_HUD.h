#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_HUD.generated.h"

class UCUserWidget_PlayerInfo;
class UCUserWidget_EquipMenu;
class UCUserWidget_Interaction;

UCLASS()
class YJJACTIONCPPUE5_API UCUserWidget_HUD : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void SetChildren();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		TObjectPtr<UCUserWidget_PlayerInfo> PlayerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
		TObjectPtr<UCUserWidget_EquipMenu> EquipMenu;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
		TObjectPtr<UCUserWidget_Interaction> Interaction;
};
