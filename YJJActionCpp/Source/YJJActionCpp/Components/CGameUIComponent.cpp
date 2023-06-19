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

	GameMode = Cast<ACGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	if (!!GameMode.Get())
	{
		TWeakObjectPtr<UCUserWidget_HUD> hud = GameMode->GetHUD();
		if (hud.Get())
		{
			hud->SetChildren();

			EquipMenu = hud->EquipMenu;

			if (!!EquipMenu.Get())
			{
				for (auto& elem : EquipMenu->EquipMenuButtons)
				{
					elem->OnWeaponEquipped.AddUniqueDynamic(this, &UCGameUIComponent::OnWeaponEquipped);
					EquipMenuButtons.Push(elem);
				}
			}
		}
	}
}

void UCGameUIComponent::OnWeaponEquipped(const EWeaponType InNewType)
{
	ACPlayableCharacter* player = Cast<ACPlayableCharacter>(Owner);
	CheckNull(player);

	UCWeaponComponent* weaponComp = CHelpers::GetComponent<UCWeaponComponent>(player);
	CheckNull(weaponComp);

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