#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/CInterface_CharacterUI.h"
#include "CGameUIComponent.generated.h"

class ACCommonCharacter;
class UCUserWidget_EquipMenu;
class UCUserWidget_EquipMenuButton;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCGameUIComponent :
	public UActorComponent,
	public ICInterface_CharacterUI
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "UI")
		TSubclassOf<UCUserWidget_EquipMenu> EquipMenuClass;

	UPROPERTY(VisibleAnywhere, Category = "UI")
		UCUserWidget_EquipMenu* EquipMenu;

public:	
	UCGameUIComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void OnMenu() override;
	void ActivateEquipMenu() override;
	void DeactivateEquipMenu() override;
	void HoveredEquipMenu() override;
	void UnhoveredEquipMenu() override;
	void ClickedEquipMenu() override;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
};
