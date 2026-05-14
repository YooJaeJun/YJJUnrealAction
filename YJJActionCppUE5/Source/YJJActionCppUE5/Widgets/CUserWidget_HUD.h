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
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetChildren();

	UFUNCTION(BlueprintPure, Category = "HUD")
	UCUserWidget_PlayerInfo* GetPlayerInfoWidget() const { return PlayerInfo; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	UCUserWidget_EquipMenu* GetEquipMenuWidget() const { return EquipMenu; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	UCUserWidget_Interaction* GetInteractionWidget() const { return Interaction; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
	TObjectPtr<UCUserWidget_PlayerInfo> PlayerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
	TObjectPtr<UCUserWidget_EquipMenu> EquipMenu;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UCUserWidget_Interaction> Interaction;
};
