#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/CInterface_CharacterUI.h"
#include "Widgets/Weapons/CUserWidget_EquipMenu.h"
#include "Widgets/Weapons/CUserWidget_EquipMenuButton.h"
#include "CGameUIComponent.generated.h"

class ACCommonCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCGameUIComponent :
	public UActorComponent,
	public ICInterface_CharacterUI
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "UI")
		TSubclassOf<UCUserWidget_EquipMenu> EquipMenuClass;

	UPROPERTY(EditAnywhere, Category = "UI")
		TSubclassOf<UCUserWidget_EquipMenuButton> EquipMenuButtonClass;

public:	
	UCGameUIComponent();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
		void OnWeaponEquipped(const EWeaponType InNewType);

public:
	void InputAction_ActivateEquipMenu() override;
	void InputAction_DeactivateEquipMenu() override;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCUserWidget_EquipMenu> EquipMenu;
	TWeakObjectPtr<UCUserWidget_EquipMenuButton> EquipMenuButton;
	TWeakObjectPtr<APlayerController> PlayerController;
};
