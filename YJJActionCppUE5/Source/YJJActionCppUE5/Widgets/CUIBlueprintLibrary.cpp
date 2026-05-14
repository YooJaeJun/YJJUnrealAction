#include "Widgets/CUIBlueprintLibrary.h"
#include "Commons/CPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/CLog.h"
#include "Widgets/CUserWidget_HUD.h"

UCUserWidget_HUD* UCUIBlueprintLibrary::EnsureLocalHUD(const UObject* WorldContextObject)
{
	if (false == IsValid(WorldContextObject))
	{
		CLog::Log(TEXT("[UI] EnsureLocalHUD: WorldContextObject 가 유효하지 않습니다."));
		return nullptr;
	}

	ACPlayerController* playerController = Cast<ACPlayerController>(
		UGameplayStatics::GetPlayerController(WorldContextObject, 0));
	if (false == IsValid(playerController))
	{
		CLog::Log(TEXT("[UI] EnsureLocalHUD: PlayerController 없음 또는 ACPlayerController 가 아님 (PlayerIndex=0)."));
		return nullptr;
	}

	return playerController->EnsureHUD();
}

UCUserWidget_HUD* UCUIBlueprintLibrary::GetLocalHUD(const UObject* WorldContextObject)
{
	if (false == IsValid(WorldContextObject))
	{
		CLog::Log(TEXT("[UI] GetLocalHUD: WorldContextObject 가 유효하지 않습니다."));
		return nullptr;
	}

	ACPlayerController* playerController = Cast<ACPlayerController>(
		UGameplayStatics::GetPlayerController(WorldContextObject, 0));
	if (false == IsValid(playerController))
	{
		CLog::Log(TEXT("[UI] GetLocalHUD: PlayerController 없음 또는 ACPlayerController 가 아님 (PlayerIndex=0)."));
		return nullptr;
	}

	return playerController->GetYJJHUD();
}
