#include "Component/CGameUIComponent.h"
#include "Global.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/GridPanel.h"
#include "Character/CCommonCharacter.h"
#include "Player/CPlayableCharacter.h"

UCGameUIComponent::UCGameUIComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());

	CHelpers::GetClass<UCUserWidget_EquipMenu>(&EquipMenuClass, "WidgetBlueprint'/Game/Widgets/Weapons/WB_CUserWidget_EquipMenu.WB_CUserWidget_EquipMenu_C'");
	CHelpers::GetClass<UCUserWidget_EquipMenuButton>(&EquipMenuButtonClass, "WidgetBlueprint'/Game/Widgets/Weapons/WB_CUserWidget_EquipMenuButton.WB_CUserWidget_EquipMenuButton_C'");
}

void UCGameUIComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!!EquipMenuClass)
	{
		EquipMenu = CreateWidget<UCUserWidget_EquipMenu, APlayerController>(Owner->GetController<APlayerController>(), EquipMenuClass);
		EquipMenu->AddToViewport();
		EquipMenu->SetVisibility(ESlateVisibility::Collapsed);
	}

	EquipMenu->OnWeaponEquipped.AddDynamic(this, &UCGameUIComponent::OnWeaponEquipped);

	PlayerController = Cast<APlayerController>(Owner->GetController());
}

void UCGameUIComponent::OnWeaponEquipped(const EWeaponType InNewType)
{
	ACPlayableCharacter* player = Cast<ACPlayableCharacter>(Owner);

	UCWeaponComponent* weaponComponent = Cast<UCWeaponComponent>(player->GetComponentByClass(UCWeaponComponent::StaticClass()));

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