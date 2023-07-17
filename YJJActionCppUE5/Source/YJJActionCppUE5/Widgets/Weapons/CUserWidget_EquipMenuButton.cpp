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
	CurWeaponType = YJJHelpers::ConvertTCHARToEnum<CEWeaponType>(GetName()[GetName().Len() - 1]);

	OnWeaponTypeHovered.ExecuteIfBound(CurWeaponType);
}

void UCUserWidget_EquipMenuButton::OnUnhovered()
{
}
