#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CGameMode.generated.h"

class UCUserWidget_HUD;

UCLASS()
class YJJACTIONCPP_API ACGameMode : public AGameModeBase
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "UI")
		TSubclassOf<UCUserWidget_HUD> MyHUDClass;

	UPROPERTY()
		UCUserWidget_HUD* HUD;

public:
	FORCEINLINE UCUserWidget_HUD* GetHUD() { return HUD; }

public:
	ACGameMode();

	virtual void BeginPlay() override;
};
