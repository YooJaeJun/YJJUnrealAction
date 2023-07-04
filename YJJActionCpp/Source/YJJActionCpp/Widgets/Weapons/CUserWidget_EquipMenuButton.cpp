#include "Widgets/Weapons/CUserWidget_EquipMenuButton.h"
#include "Global.h"
#include "Components/Button.h"

void UCUserWidget_EquipMenuButton::BindEquipMenuButton()
{
	Button = Cast<UButton>(GetWidgetFromName(TEXT("CButton")));
	Button->OnHovered.AddUniqueDynamic(this, &UCUserWidget_EquipMenuButton::OnHovered);
	Button->OnUnhovered.AddUniqueDynamic(this, &UCUserWidget_EquipMenuButton::OnUnhovered);
}

void UCUserWidget_EquipMenuButton::OnHovered()
{
	const EWeaponType weaponType =
		YJJHelpers::ConvertTCHARToEnum<EWeaponType>(GetName()[GetName().Len() - 1]);

	if (OnWeaponEquipped.IsBound())
		OnWeaponEquipped.Broadcast(weaponType);
}

void UCUserWidget_EquipMenuButton::OnUnhovered()
{
}
