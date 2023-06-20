#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/CInterface_CharacterUI.h"
#include "Widgets/Weapons/CUserWidget_EquipMenu.h"
#include "Widgets/Weapons/CUserWidget_EquipMenuButton.h"
#include "CGameUIComponent.generated.h"

class ACCommonCharacter;
class UUserWidget;
class ACGameMode;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCGameUIComponent :
	public UActorComponent,
	public ICInterface_CharacterUI
{
	GENERATED_BODY()

public:
	UCGameUIComponent();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
		void OnWeaponEquipped(const EWeaponType InNewType);

public:
	virtual void InputAction_ActivateEquipMenu() override;
	virtual void InputAction_DeactivateEquipMenu() override;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCUserWidget_EquipMenu> EquipMenu;
	TArray<TWeakObjectPtr<UCUserWidget_EquipMenuButton>> EquipMenuButtons;
	TWeakObjectPtr<APlayerController> PlayerController;
};
