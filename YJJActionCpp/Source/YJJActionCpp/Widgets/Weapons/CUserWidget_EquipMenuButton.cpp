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
	const TCHAR prevLast = GetName()[GetName().Len() - 2];
	const TCHAR last = GetName()[GetName().Len() - 1];
	FString WeaponStr;

	if (prevLast >= '0' && prevLast <= '9')
		WeaponStr += prevLast;

	WeaponStr += last;

	CurWeaponType = YJJHelpers::ConvertFStringToEnum<CEWeaponType>(WeaponStr);

	if (false == OnWeaponTypeHovered.ExecuteIfBound(CurWeaponType))
		CLog::Log("OnWeaponTypeHovered Failed");
}

void UCUserWidget_EquipMenuButton::OnUnhovered()
{
}
