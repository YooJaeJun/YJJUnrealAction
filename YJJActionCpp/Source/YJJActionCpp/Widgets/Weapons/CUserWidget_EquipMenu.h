#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Component/CWeaponComponent.h"
#include "CUserWidget_EquipMenu.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponEquipped, const EWeaponType, InNewType);

UCLASS()
class YJJACTIONCPP_API UCUserWidget_EquipMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
		FWeaponEquipped OnWeaponEquipped;

public:
	void Activate(const float TimeDilation);
	void Deactivate(const float TimeDilation);

private:
	FString ButtonInHovering;
};
