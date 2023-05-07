#include "Game/CGameMode.h"
#include "Global.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/CUserWidget_HUD.h"

ACGameMode::ACGameMode()
{
    CHelpers::GetClass<APawn>(&DefaultPawnClass, "Blueprint'/Game/Player/BP_CPlayableCharacter.BP_CPlayableCharacter_C'");
    CHelpers::GetClass<UCUserWidget_HUD>(&PlayerHUDClass, "WidgetBlueprint'/Game/Widgets/WB_CUserWidget_HUD.WB_CUserWidget_HUD_C'");
}

void ACGameMode::BeginPlay()
{
	Super::BeginPlay();

    if (!!PlayerHUDClass)
    {
        PlayerHUD = CreateWidget<UCUserWidget_HUD>(GetWorld(), PlayerHUDClass);

        if (PlayerHUD)
			PlayerHUD->AddToViewport();
    }
}
