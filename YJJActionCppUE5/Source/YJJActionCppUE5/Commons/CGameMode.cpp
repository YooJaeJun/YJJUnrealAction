#include "Global.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/CUserWidget_HUD.h"

ACGameMode::ACGameMode()
{
    YJJHelpers::GetClass<APawn>(&DefaultPawnClass, "/Script/Engine.Blueprint'/Game/Character/Player/CBP_PlayableCharacter.CBP_PlayableCharacter_C'");
    YJJHelpers::GetClass<UCUserWidget_HUD>(&PlayerHUDClass, "/Script/UMGEditor.WidgetBlueprint'/Game/Widgets/CWB_HUD.CWB_HUD_C'");
}

void ACGameMode::BeginPlay()
{
	Super::BeginPlay();

    if (IsValid(PlayerHUDClass))
    {
        PlayerHUD = CreateWidget<UCUserWidget_HUD>(GetWorld(), PlayerHUDClass);
        
        if (IsValid(PlayerHUD))
			PlayerHUD->AddToViewport();
    }
}
