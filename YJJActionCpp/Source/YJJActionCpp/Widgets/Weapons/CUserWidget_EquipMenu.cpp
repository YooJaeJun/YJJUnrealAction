#include "Widgets/Weapons/CUserWidget_EquipMenu.h"
#include "Global.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "CUserWidget_EquipMenuButton.h"

void UCUserWidget_EquipMenu::BindChildren()
{
	for (int i=0; ; i++)
	{
		FString buttonStr = "Action_";
		buttonStr += FString::FromInt(i);
		FName buttonName = FName(*buttonStr);

		const TWeakObjectPtr<UCUserWidget_EquipMenuButton> button = 
			Cast<UCUserWidget_EquipMenuButton>(GetWidgetFromName(buttonName));

		CheckNull(button);

		button->BindEquipMenuButton();
		button->OnWeaponTypeHovered.BindUFunction(this, "SetWeaponType");

		EquipMenuButtons.Add(button.Get());
	}
}

void UCUserWidget_EquipMenu::Activate(const float TimeDilation)
{
	SetVisibility(ESlateVisibility::Visible);

	if (!!GetOwningPlayer())
	{
		GetOwningPlayer()->SetShowMouseCursor(true);
		UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(
			GetOwningPlayer(), 
			nullptr,
			EMouseLockMode::LockInFullscreen, 
			true);
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeDilation);
	}
}

void UCUserWidget_EquipMenu::Deactivate(const float TimeDilation)
{
	SetVisibility(ESlateVisibility::Collapsed);

	CheckNull(GetOwningPlayer());

	GetOwningPlayer()->SetShowMouseCursor(false);

	UWidgetBlueprintLibrary::SetInputMode_GameOnly(GetOwningPlayer());

	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeDilation);

	if (OnWeaponEquipped.IsBound())
		OnWeaponEquipped.Broadcast(CurWeaponType);
}

void UCUserWidget_EquipMenu::SetWeaponType(const CEWeaponType InNewType)
{
	CurWeaponType = InNewType;
}
