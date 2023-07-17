#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CGameMode.generated.h"

class UCUserWidget_HUD;

UCLASS()
class YJJACTIONCPPUE5_API ACGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACGameMode();

	virtual void BeginPlay() override;

public:
	FORCEINLINE TObjectPtr<UCUserWidget_HUD> GetHUD() const { return PlayerHUD; }

private:
	UPROPERTY(EditAnywhere, Category = "UI")
		TSubclassOf<UCUserWidget_HUD> PlayerHUDClass;

	UPROPERTY(EditAnywhere, Category = "UI")
		TObjectPtr<UCUserWidget_HUD> PlayerHUD;
};
