#include "Components/CGameUIComponent.h"
#include "Global.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/GridPanel.h"
#include "Components/WidgetComponent.h"
#include "Character/CCommonCharacter.h"
#include "Player/CPlayableCharacter.h"
#include "Blueprint/UserWidget.h"

UCGameUIComponent::UCGameUIComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());

	CHelpers::GetClass<UCUserWidget_EquipMenu>(
		&EquipMenuClass, "WidgetBlueprint'/Game/Widgets/Weapons/WB_CEquipMenu.WB_CEquipMenu_C'");
	CHelpers::GetClass<UCUserWidget_EquipMenuButton>(
		&EquipMenuButtonClass, "WidgetBlueprint'/Game/Widgets/Weapons/WB_CEquipMenuButton.WB_CEquipMenuButton_C'");

	if (!!EquipMenuClass)
	{
		EquipMenu = CreateWidget<UCUserWidget_EquipMenu, APlayerController>(
			Owner->GetController<APlayerController>(), EquipMenuClass);
		EquipMenu->AddToViewport();
		EquipMenu->SetVisibility(ESlateVisibility::Collapsed);

		if (!!EquipMenuButtonClass)
		{
			for (int i=0; i<EquipMenu->EquipMenuButtons.Num(); i++)
				EquipMenuButtons[i] = EquipMenu->EquipMenuButtons[i];
		}
	}

	for (auto& elem : EquipMenuButtons)
		elem->OnWeaponEquipped.AddDynamic(this, &UCGameUIComponent::OnWeaponEquipped);
}

void UCGameUIComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(Owner->GetController());
}

void UCGameUIComponent::OnWeaponEquipped(const EWeaponType InNewType)
{
	ACPlayableCharacter* player = Cast<ACPlayableCharacter>(Owner);

	UCWeaponComponent* weaponComponent = Cast<UCWeaponComponent>(
		player->GetComponentByClass(UCWeaponComponent::StaticClass()));

	weaponComponent->SetMode(InNewType);
}

void UCGameUIComponent::InputAction_ActivateEquipMenu()
{
	EquipMenu->Activate(0.1f);
}

void UCGameUIComponent::InputAction_DeactivateEquipMenu()
{
	EquipMenu->Deactivate(1.0f);
}