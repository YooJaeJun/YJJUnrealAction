#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "Components/CWeaponComponent.h"
#include "CUserWidget_EquipMenuButton.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponEquipped, const EWeaponType, InNewType);

UCLASS(Abstract)
class YJJACTIONCPP_API UCUserWidget_EquipMenuButton : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindEquipMenuButton();

	void OnHovered();
	void OnUnhovered();

public:
	UPROPERTY()
		UButton* Button;

	UPROPERTY()
		FWeaponEquipped OnWeaponEquipped;
};
