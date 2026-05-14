#include "Widgets/CUIBlueprintLibrary.h"
#include "Commons/CPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/CUserWidget_HUD.h"

UCUserWidget_HUD* UCUIBlueprintLibrary::EnsureLocalHUD(const UObject* WorldContextObject)
{
	if (false == IsValid(WorldContextObject))
		return nullptr;

	ACPlayerController* playerController = Cast<ACPlayerController>(
		UGameplayStatics::GetPlayerController(WorldContextObject, 0));
	if (false == IsValid(playerController))
		return nullptr;

	return playerController->EnsureHUD();
}

UCUserWidget_HUD* UCUIBlueprintLibrary::GetLocalHUD(const UObject* WorldContextObject)
{
	if (false == IsValid(WorldContextObject))
		return nullptr;

	ACPlayerController* playerController = Cast<ACPlayerController>(
		UGameplayStatics::GetPlayerController(WorldContextObject, 0));
	if (false == IsValid(playerController))
		return nullptr;

	return playerController->GetYJJHUD();
}
