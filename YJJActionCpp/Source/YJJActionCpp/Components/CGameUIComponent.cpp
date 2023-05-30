#include "Components/CGameUIComponent.h"
#include "Global.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Character/CCommonCharacter.h"
#include "Player/CPlayableCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Game/CGameMode.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Game/CGameMode.h"

UCGameUIComponent::UCGameUIComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCGameUIComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(Owner->GetController());

	GameMode = Cast<ACGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	if (!!GameMode)
	{
		TWeakObjectPtr<UCUserWidget_HUD> hud = GameMode->GetHUD();
		if (hud.Get())
		{
			hud->SetChild();

			EquipMenu = hud->EquipMenu;

			if (!!EquipMenu.Get())
			{
				for (auto& elem : EquipMenu->EquipMenuButtons)
				{
					elem->OnWeaponEquipped.AddDynamic(this, &UCGameUIComponent::OnWeaponEquipped);
					EquipMenuButtons.Push(elem);
				}
			}
		}
	}
}

void UCGameUIComponent::OnWeaponEquipped(const EWeaponType InNewType)
{
	ACPlayableCharacter* player = Cast<ACPlayableCharacter>(Owner);

	UCWeaponComponent* weaponComponent = CHelpers::GetComponent<UCWeaponComponent>(player);

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