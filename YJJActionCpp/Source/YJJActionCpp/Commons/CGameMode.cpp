#include "Commons/CGameMode.h"
#include "Global.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/CUserWidget_HUD.h"

ACGameMode::ACGameMode()
{
    YJJHelpers::GetClass<APawn>(&DefaultPawnClass, "Blueprint'/Game/Character/Player/CBP_PlayableCharacter.CBP_PlayableCharacter_C'");
    YJJHelpers::GetClass<UCUserWidget_HUD>(&PlayerHUDClass, "WidgetBlueprint'/Game/Widgets/CWB_HUD.CWB_HUD_C'");
}

void ACGameMode::BeginPlay()
{
	Super::BeginPlay();

    if (!!PlayerHUDClass)
    {
        PlayerHUD = CreateWidget<UCUserWidget_HUD>(GetWorld(), PlayerHUDClass);
        
        if (!!PlayerHUD)
			PlayerHUD->AddToViewport();
    }
}
