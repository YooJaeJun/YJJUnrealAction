#pragma once

#include "CoreMinimal.h"
#include "Commons/CEnums.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_EquipMenuButton.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FWeaponTypeHovered, const CEWeaponType, InNewType);

class UButton;

UCLASS(Abstract)
class YJJACTIONCPP_API UCUserWidget_EquipMenuButton : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindEquipMenuButton();

	UFUNCTION()
		void OnHovered();

	UFUNCTION()
		void OnUnhovered();

public:
	UPROPERTY()
		UButton* Button;

	UPROPERTY()
		FWeaponTypeHovered OnWeaponTypeHovered;

public:
	CEWeaponType CurWeaponType;
};
