#pragma once

#include "CoreMinimal.h"
#include "Commons/CEnums.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_EquipMenuButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponTypeHoveredMulticast, const CEWeaponType, InNewType);

class UButton;

UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API UCUserWidget_EquipMenuButton : public UCUserWidget_Custom
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
	TObjectPtr<UButton> Button;

	UPROPERTY(BlueprintAssignable, Category = "EquipMenu")
	FWeaponTypeHoveredMulticast OnWeaponTypeHovered;

	UPROPERTY(BlueprintAssignable, Category = "EquipMenu")
	FWeaponTypeHoveredMulticast OnWeaponTypeUnhovered;

public:
	CEWeaponType CurWeaponType;
};
