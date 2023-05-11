#include "Widgets/Weapons/CUserWidget_EquipMenu.h"
#include "Global.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "CUserWidget_EquipMenuButton.h"

void UCUserWidget_EquipMenu::SetChild()
{
	for (int i=0; ; i++)
	{
		FString buttonStr = "Action_";
		buttonStr += FString::FromInt(i);
		FName buttonName = FName(*buttonStr);

		UCUserWidget_EquipMenuButton* button = Cast<UCUserWidget_EquipMenuButton>(GetWidgetFromName(buttonName));

		if (nullptr == button)
			break;

		button->BindEquipMenuButton();
		EquipMenuButtons.Add(button);
	}
}

void UCUserWidget_EquipMenu::Activate(const float TimeDilation)
{
	SetVisibility(ESlateVisibility::Visible);

	if (!!GetOwningPlayer())
	{
		GetOwningPlayer()->SetShowMouseCursor(true);
		UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(GetOwningPlayer(), nullptr,
			EMouseLockMode::LockInFullscreen, true);
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeDilation);
	}
}

void UCUserWidget_EquipMenu::Deactivate(const float TimeDilation)
{
	SetVisibility(ESlateVisibility::Collapsed);

	if (!!GetOwningPlayer())
	{
		GetOwningPlayer()->SetShowMouseCursor(false);
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(GetOwningPlayer());
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeDilation);
	}
}