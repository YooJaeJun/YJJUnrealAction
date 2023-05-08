#include "Widgets/Weapons/CUserWidget_EquipMenuButton.h"
#include "Global.h"
#include "Components/Button.h"

void UCUserWidget_EquipMenuButton::BindEquipMenuButton()
{
	Button = Cast<UButton>(GetWidgetFromName(TEXT("CButton")));
	// Button->OnHovered.AddDynamic(this, &OnHovered);
	// Button->OnUnhovered.AddDynamic(this, &OnUnhovered);

	EWeaponType weaponType = CHelpers::ConvertTCHARToEnum<EWeaponType>(GetName()[GetName().Len() - 1]);

	if (OnWeaponEquipped.IsBound())
		OnWeaponEquipped.Broadcast(weaponType);
}

void UCUserWidget_EquipMenuButton::OnHovered()
{
}

void UCUserWidget_EquipMenuButton::OnUnhovered()
{
}
