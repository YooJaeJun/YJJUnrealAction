#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "Components/CWeaponComponent.h"
#include "CUserWidget_EquipMenu.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponEquipped, const EWeaponType, InNewType);

UCLASS()
class YJJACTIONCPP_API UCUserWidget_EquipMenu : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void Activate(const float TimeDilation);
	void Deactivate(const float TimeDilation);

public:
	UPROPERTY()
		FWeaponEquipped OnWeaponEquipped;

private:
	FString ButtonInHovering;
};
