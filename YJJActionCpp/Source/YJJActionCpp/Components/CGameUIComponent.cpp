#include "Components/CGameUIComponent.h"
#include "Global.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Widgets/Weapons/CUserWidget_CrossHair.h"

UCGameUIComponent::UCGameUIComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCGameUIComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(Owner->GetController());

	const TWeakObjectPtr<UCUserWidget_HUD> hud = YJJHelpers::GetHud(Owner);
	CheckNull(hud.Get());

	hud->SetChildren();

	EquipMenu = hud->EquipMenu;
	CheckNull(EquipMenu.Get());

	EquipMenu->OnWeaponEquipped.AddUniqueDynamic(this, &UCGameUIComponent::OnWeaponEquipped);

	CrossHair = hud->CrossHair;
	CheckNull(CrossHair.Get());
}

void UCGameUIComponent::OnWeaponEquipped(const CEWeaponType InNewType)
{
	const TWeakObjectPtr<ACPlayableCharacter> player = Cast<ACPlayableCharacter>(Owner);
	CheckNull(player.Get());

	const TWeakObjectPtr<UCWeaponComponent> weaponComp = YJJHelpers::GetComponent<UCWeaponComponent>(player.Get());
	CheckNull(weaponComp.Get());

	weaponComp->SetMode(InNewType);
}

void UCGameUIComponent::InputAction_ActivateEquipMenu()
{
	CheckNull(EquipMenu);
	EquipMenu->Activate(0.1f);
}

void UCGameUIComponent::InputAction_DeactivateEquipMenu()
{
	CheckNull(EquipMenu);
	EquipMenu->Deactivate(1.0f);
}

void UCGameUIComponent::ActivateCrossHair()
{
	CheckNull(CrossHair);
	CrossHair->SetVisibility(ESlateVisibility::Visible);
}

void UCGameUIComponent::DeactivateCrossHair()
{
	CheckNull(CrossHair);
	CrossHair->SetVisibility(ESlateVisibility::Collapsed);
}

void UCGameUIComponent::SetColor_Red() const
{
	CrossHair->SetColor_Red();
}

void UCGameUIComponent::SetColor_White() const
{
	CrossHair->SetColor_White();
}
