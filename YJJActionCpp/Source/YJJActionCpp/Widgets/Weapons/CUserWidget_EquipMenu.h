#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "Components/CWeaponComponent.h"
#include "CUserWidget_EquipMenu.generated.h"

class UCUserWidget_EquipMenuButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponEquipped, const EWeaponType, InNewType);

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
	UFUNCTION()
		void SetWeaponType(const EWeaponType InNewType);

public:
	UPROPERTY()
		TArray<UCUserWidget_EquipMenuButton*> EquipMenuButtons;

	UPROPERTY()
		FString ButtonHovered;

	UPROPERTY()
		FWeaponEquipped OnWeaponEquipped;

public:
	EWeaponType CurWeaponType;
};
