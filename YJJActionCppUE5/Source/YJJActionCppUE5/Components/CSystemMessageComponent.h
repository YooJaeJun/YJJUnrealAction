#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Internationalization/Text.h"
#include "CSystemMessageComponent.generated.h"

class UUserWidget;

// BP SystemMessageComponent — HUD 의 WB_Message 에 Print 후 Hide 타이머로 HideMessage 에 가깝게 동작한다.
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YJJACTIONCPPUE5_API UCSystemMessageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCSystemMessageComponent();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// BP 의 Play(InText, InTime) 노드 이름·타입 매칭(프로세스 이벤트 호환 포함).
	UFUNCTION(BlueprintCallable, Category = "UI|SystemMessage")
	void Play(const FText& InText, double InTime);

	// BP 타이머가 이름으로 호출 — HideMessage 는 HUD 의 WB_Message 로 전달된다.
	UFUNCTION(BlueprintCallable, Category = "UI|SystemMessage")
	void Hide();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|SystemMessage", meta = (DisplayName = "Message UI"))
	TObjectPtr<UUserWidget> MessageUI;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|SystemMessage", meta = (DisplayName = "Message Widget Class"))
	TSubclassOf<UUserWidget> MessageWidgetClass;

private:
	void ResolveOrCreateOwnedMessageWidgetLazy(APlayerController* InOwningPlayerResolved);

	// BP 의 GetGameplayStatics:GetGameMode → GetHUD → WB_Message; GameMode 미존재 시 EnsureHUD 폴백.
	UUserWidget* ResolveHudMessageWidgetOrNull();

	static void DispatchPrintMessage(UUserWidget* InTargetWidget, const FText& InText);

	static void DispatchHideMessage(UUserWidget* InTargetWidget);

	static void InvokeUserWidgetVoid(UUserWidget* InWidget, const FName& InFunctionName);

	void ClearScheduledHideTimer();

	FTimerHandle HideTimerHandle;
};
