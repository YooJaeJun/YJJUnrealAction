#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CGameMode.generated.h"

class UCUserWidget_HUD;
class UWidgetComponent;

UCLASS()
class YJJACTIONCPP_API ACGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACGameMode();

	virtual void BeginPlay() override;

public:
	FORCEINLINE UCUserWidget_HUD* GetHUD() const { return PlayerHUD; }

private:
	UPROPERTY(EditAnywhere, Category = "UI")
		TSubclassOf<UCUserWidget_HUD> PlayerHUDClass;

	UPROPERTY(EditAnywhere, Category = "UI")
		UCUserWidget_HUD* PlayerHUD;
};
