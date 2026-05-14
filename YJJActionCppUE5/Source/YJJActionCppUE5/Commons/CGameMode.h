#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Widgets/CUserWidget_HUD.h"
#include "CGameMode.generated.h"

UCLASS()
class YJJACTIONCPPUE5_API ACGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACGameMode();

	virtual void BeginPlay() override;

	// 블루프린트/레거시 경로 호환: 예전처럼 GameMode에서 HUD 포인터를 들고 있지 않고,
	// 로컬 ACPlayerController::EnsureHUD() 결과를 돌려준다.
	UFUNCTION(BlueprintPure, Category = "UI")
	UCUserWidget_HUD* GetHUD() const;
};
