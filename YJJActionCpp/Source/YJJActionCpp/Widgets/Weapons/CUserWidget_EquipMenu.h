#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_EquipMenu.generated.h"

class UCUserWidget_EquipMenuButton;

UCLASS()
class YJJACTIONCPP_API UCUserWidget_EquipMenu : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindChildren();

public:
	void Activate(const float TimeDilation);
	void Deactivate(const float TimeDilation);

public:
	UPROPERTY()
		TArray<UCUserWidget_EquipMenuButton*> EquipMenuButtons;

	UPROPERTY()
		FString ButtonHovered;
};
