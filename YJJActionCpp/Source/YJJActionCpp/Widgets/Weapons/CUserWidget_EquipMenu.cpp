#include "Widgets/Weapons/CUserWidget_EquipMenu.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Component/CGameUIComponent.h"
#include "Kismet/GameplayStatics.h"

void UCUserWidget_EquipMenu::Activate(const float TimeDilation)
{
	SetVisibility(ESlateVisibility::Visible);

	if (!!GetOwningPlayer())
	{
		GetOwningPlayer()->SetShowMouseCursor(true);
		UWidgetBlueprintLibrary::SetInputMode_GameAndUI(GetOwningPlayer(), nullptr,
			true, true);
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

	if (OnWeaponEquipped.IsBound())
		OnWeaponEquipped.Broadcast(EWeaponType::Sword);
}