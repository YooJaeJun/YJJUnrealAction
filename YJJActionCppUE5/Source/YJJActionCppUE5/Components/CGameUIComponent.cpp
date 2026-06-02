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

	Owner = Cast<ACCommonCharacter>(GetOwner());

	// AI·네트 리모트 폰 등에는 로컬 HUD 가 없음 — 타이밍 순서 때문에 스팸 경고 내지 않는다.
	if (false == Owner.IsValid() || false == Owner->IsLocallyControlled())
		return;

	PlayerController = Cast<APlayerController>(Owner->GetController());
	if (false == PlayerController.IsValid() || false == PlayerController->IsLocalController())
	{
		const FString ownerName = Owner.IsValid() ? Owner->GetName() : TEXT("(Owner 무효)");
		CLog::Log(FString::Printf(TEXT("[UI] CGameUIComponent::BeginPlay: 로컬 폰인데 PlayerController 무효 또는 네트워크 비로컬 — Owner=%s"), *ownerName));
		return;
	}

	const TWeakObjectPtr<ACPlayerController> yjjPlayerController = Cast<ACPlayerController>(PlayerController.Get());
	CheckNull(yjjPlayerController.Get());

	const TWeakObjectPtr<UCUserWidget_HUD> hud = yjjPlayerController->EnsureHUD();
	CheckNull(hud.Get());

	hud->SetChildren();

	EquipMenu = hud->GetEquipMenuWidget();
	if (false == EquipMenu.IsValid())
		CLog::Log(FString::Printf(TEXT("[UI] CGameUIComponent::BeginPlay: EquipMenu 없음 — Owner=%s"),
			Owner.IsValid() ? *Owner->GetName() : TEXT("(무효)")));

	const TWeakObjectPtr<ACPlayableCharacter> playable = Cast<ACPlayableCharacter>(Owner.Get());
	if (playable.IsValid())
	{
		playable->SetMenuUI();
		RefreshEquipMenuFromHud();
	}
	else
		CLog::Log(TEXT("[UI] CGameUIComponent::BeginPlay: ACPlayableCharacter 아님 — SetMenuUI 생략"));
}

void UCGameUIComponent::RefreshEquipMenuFromHud()
{
	if (false == Owner.IsValid() || false == Owner->IsLocallyControlled())
	{
		return;
	}

	APlayerController* playerControllerPtr = PlayerController.Get();
	if (false == IsValid(playerControllerPtr))
	{
		playerControllerPtr = Cast<APlayerController>(Owner->GetController());
	}

	ACPlayerController* const yjjPlayerController = Cast<ACPlayerController>(playerControllerPtr);
	if (false == IsValid(yjjPlayerController))
	{
		return;
	}

	UCUserWidget_HUD* const hud = yjjPlayerController->EnsureHUD();
	if (false == IsValid(hud))
	{
		return;
	}

	hud->SetChildren();

	UCUserWidget_EquipMenu* const resolvedEquipMenu = hud->GetEquipMenuWidget();
	if (IsValid(resolvedEquipMenu))
	{
		EquipMenu = resolvedEquipMenu;
	}
}

void UCGameUIComponent::InputAction_ActivateEquipMenu()
{
	if (false == EquipMenu.IsValid())
	{
		RefreshEquipMenuFromHud();
	}

	CheckNull(EquipMenu);
	EquipMenu->Activate(0.1f);
}

void UCGameUIComponent::InputAction_DeactivateEquipMenu()
{
	if (false == EquipMenu.IsValid())
	{
		RefreshEquipMenuFromHud();
	}

	CheckNull(EquipMenu);
	EquipMenu->Deactivate(1.0f);
}