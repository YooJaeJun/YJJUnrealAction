#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "Components/CWeaponComponent.h"
#include "CUserWidget_EquipMenu.generated.h"

class UCUserWidget_EquipMenuButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponEquipped, const CEWeaponType, InNewType);

UCLASS()
class YJJACTIONCPPUE5_API UCUserWidget_EquipMenu : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindChildren();

public:
	void Activate(const float TimeDilation);
	void Deactivate(const float TimeDilation);

public:
	UFUNCTION()
		void SetWeaponType(const CEWeaponType InNewType);

public:
	UPROPERTY()
		TArray<TObjectPtr<UCUserWidget_EquipMenuButton>> EquipMenuButtons;

	UPROPERTY()
		FString ButtonHovered;

	UPROPERTY()
		FWeaponEquipped OnWeaponEquipped;

public:
	CEWeaponType CurWeaponType;
};
