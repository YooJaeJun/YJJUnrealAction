#include "Component/CGameUIComponent.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "Widgets/Weapons/CUserWidget_EquipMenu.h"
#include "Widgets/Weapons/CUserWidget_EquipMenuButton.h"

UCGameUIComponent::UCGameUIComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());

	CHelpers::GetClass<UCUserWidget_EquipMenu>(&EquipMenuClass, "WidgetBlueprint'/Game/Widgets/Weapons/WB_CUserWidget_EquipMenu.WB_CUserWidget_EquipMenu_C'");
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
}

void UCGameUIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCGameUIComponent::OnMenu()
{
	ActivateEquipMenu();
}

void UCGameUIComponent::ActivateEquipMenu()
{
	EquipMenu->SetVisibility(ESlateVisibility::Visible);
}

void UCGameUIComponent::DeactivateEquipMenu()
{
}

void UCGameUIComponent::HoveredEquipMenu()
{
}

void UCGameUIComponent::UnhoveredEquipMenu()
{
}

void UCGameUIComponent::ClickedEquipMenu()
{
}
