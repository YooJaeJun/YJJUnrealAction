#include "Components/CGameUIComponent.h"
#include "Global.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Commons/CGameMode.h"
#include "Widgets/CUserWidget_HUD.h"

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