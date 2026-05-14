#include "Components/CGameUIComponent.h"
#include "Global.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Commons/CGameMode.h"
#include "Commons/CPlayerController.h"
#include "Widgets/CUserWidget_HUD.h"

UCGameUIComponent::UCGameUIComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCGameUIComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(Owner->GetController());
	if (false == PlayerController.IsValid() || false == PlayerController->IsLocalController())
	{
		const FString ownerName = Owner.IsValid() ? Owner->GetName() : TEXT("(Owner 무효)");
		CLog::Log(FString::Printf(TEXT("[UI] CGameUIComponent::BeginPlay: PlayerController 무효 또는 비로컬 — Owner=%s"), *ownerName));
		return;
	}

	const TWeakObjectPtr<ACPlayerController> yjjPlayerController = Cast<ACPlayerController>(PlayerController.Get());
	CheckNull(yjjPlayerController.Get());

	const TWeakObjectPtr<UCUserWidget_HUD> hud = yjjPlayerController->EnsureHUD();
	CheckNull(hud.Get());

	hud->SetChildren();

	EquipMenu = hud->EquipMenu;
	if (false == EquipMenu.IsValid())
		CLog::Log(FString::Printf(TEXT("[UI] CGameUIComponent::BeginPlay: EquipMenu 없음 — Owner=%s"),
			Owner.IsValid() ? *Owner->GetName() : TEXT("(무효)")));

	const TWeakObjectPtr<ACPlayableCharacter> playable = Cast<ACPlayableCharacter>(Owner.Get());
	if (playable.IsValid())
		playable->SetMenuUI();
	else
		CLog::Log(TEXT("[UI] CGameUIComponent::BeginPlay: ACPlayableCharacter 아님 — SetMenuUI 생략"));
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