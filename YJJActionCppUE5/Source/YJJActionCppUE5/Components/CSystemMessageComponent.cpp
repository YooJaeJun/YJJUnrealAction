#include "Components/CSystemMessageComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Characters/CCommonCharacter.h"
#include "Commons/CGameMode.h"
#include "Utilities/CLog.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

namespace
{
	const FName GPrintMessageName(TEXT("PrintMessage"));
	const FName GHideMessageName(TEXT("HideMessage"));
}

UCSystemMessageComponent::UCSystemMessageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	static ConstructorHelpers::FClassFinder<UUserWidget> messageClassFinder(
		TEXT("/Game/Widgets/Interaction/WB_Message.WB_Message_C"));
	if (messageClassFinder.Succeeded())
		MessageWidgetClass = messageClassFinder.Class;
}

void UCSystemMessageComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ClearScheduledHideTimer();
}

void UCSystemMessageComponent::Play(const FText& InText, double InTime)
{
	UWorld* worldResolved = GetWorld();
	if (false == IsValid(worldResolved) || NM_DedicatedServer == worldResolved->GetNetMode())
		return;

	AActor* ownerActorResolved = GetOwner();
	// 블루프린트는 소유 Actor 를 BP_Character 로 캐스트한 뒤에만 진행한다.
	if (Cast<ACCommonCharacter>(ownerActorResolved) == nullptr)
	{
		CLog::Log(FString::Printf(TEXT("[SystemMessage] Play: Owner 가 ACCommonCharacter 가 아님 — %s"),
			IsValid(ownerActorResolved) ? *ownerActorResolved->GetName() : TEXT("nullptr")));
		return;
	}

	APawn* ownerAsPawnResolved = Cast<APawn>(ownerActorResolved);
	if (IsValid(ownerAsPawnResolved) && false == ownerAsPawnResolved->IsLocallyControlled())
		return;

	APlayerController* playerControllerResolved = nullptr;
	ACCommonCharacter* ownerCharacterForControllerResolved = Cast<ACCommonCharacter>(ownerActorResolved);
	if (IsValid(ownerCharacterForControllerResolved) && IsValid(ownerCharacterForControllerResolved->CurController.Get()))
	{
		playerControllerResolved =
			Cast<APlayerController>(ownerCharacterForControllerResolved->CurController.Get());
	}

	ResolveOrCreateOwnedMessageWidgetLazy(playerControllerResolved);

	UUserWidget* hudMessageResolved = ResolveHudMessageWidgetOrNull();
	if (false == IsValid(hudMessageResolved))
	{
		CLog::Log(FString::Printf(TEXT("[SystemMessage] Play: HUD WB_Message 미해결 — Owner=%s"),
			IsValid(ownerActorResolved) ? *ownerActorResolved->GetName() : TEXT("<null>")));
		return;
	}

	DispatchPrintMessage(hudMessageResolved, InText);
	ClearScheduledHideTimer();

	const float delayResolved = static_cast<float>(InTime);
	if (delayResolved <= 0.0f)
	{
		Hide();
		return;
	}

	worldResolved->GetTimerManager().SetTimer(
		HideTimerHandle,
		FTimerDelegate::CreateUObject(this, &UCSystemMessageComponent::Hide),
		delayResolved,
		false);
}

void UCSystemMessageComponent::Hide()
{
	UUserWidget* hudMessageResolved = ResolveHudMessageWidgetOrNull();
	if (false == IsValid(hudMessageResolved))
	{
		CLog::Log(TEXT("[SystemMessage] Hide: HUD WB_Message 미해결 — HideMessage 생략"));
		ClearScheduledHideTimer();
		return;
	}

	DispatchHideMessage(hudMessageResolved);
	ClearScheduledHideTimer();
}

void UCSystemMessageComponent::ResolveOrCreateOwnedMessageWidgetLazy(APlayerController* InOwningPlayerResolved)
{
	if (IsValid(MessageUI) || MessageWidgetClass == nullptr)
		return;

	if (false == IsValid(InOwningPlayerResolved))
		return;

	UUserWidget* createdResolved =
		CreateWidget<UUserWidget>(InOwningPlayerResolved, MessageWidgetClass.Get());
	MessageUI = createdResolved;

	if (false == IsValid(createdResolved))
	{
		CLog::Log(TEXT("[SystemMessage] MessageUI CreateWidget 실패 — 클래스나 PC 확인"));
	}
}

UUserWidget* UCSystemMessageComponent::ResolveHudMessageWidgetOrNull()
{
	AActor* ownerActorResolved = GetOwner();
	const UObject* worldContextResolved = IsValid(ownerActorResolved)
		? static_cast<const UObject*>(ownerActorResolved)
		: static_cast<const UObject*>(this);

	// BP Play/Hide 의 GetGameplayStatics:GetGameMode → BP_GameMode::GetHUD → WB_Message 경로 우선.
	UCUserWidget_HUD* hudResolved = nullptr;
	ACGameMode* gameModeResolved = Cast<ACGameMode>(UGameplayStatics::GetGameMode(worldContextResolved));
	if (IsValid(gameModeResolved))
		hudResolved = gameModeResolved->GetHUD();

	// 순 클라 등 GameMode 객체가 없으면 레거시 ACCommonCharacter::EnsureHUD 가 동일 목적이다.
	if (false == IsValid(hudResolved))
	{
		ACCommonCharacter* ownerCharacterResolved = Cast<ACCommonCharacter>(ownerActorResolved);
		if (IsValid(ownerCharacterResolved))
			hudResolved = ownerCharacterResolved->EnsureHUDWidget();
	}

	if (false == IsValid(hudResolved))
		return nullptr;

	return hudResolved->ResolveWB_MessageWidget();
}

void UCSystemMessageComponent::DispatchPrintMessage(UUserWidget* InTargetWidget, const FText& InText)
{
	if (false == IsValid(InTargetWidget))
		return;

	UFunction* functionResolved = InTargetWidget->FindFunction(GPrintMessageName);
	if (functionResolved == nullptr)
	{
		CLog::Log(FString::Printf(TEXT("[SystemMessage] PrintMessage 없음 — %s"),
			*InTargetWidget->GetClass()->GetName()));
		return;
	}

	struct FPayload
	{
		FText InText;
	} Payload{ InText };
	InTargetWidget->ProcessEvent(functionResolved, &Payload);
}

void UCSystemMessageComponent::DispatchHideMessage(UUserWidget* InTargetWidget)
{
	if (false == IsValid(InTargetWidget))
		return;

	InvokeUserWidgetVoid(InTargetWidget, GHideMessageName);
}

void UCSystemMessageComponent::InvokeUserWidgetVoid(UUserWidget* InWidget, const FName& InFunctionName)
{
	if (false == IsValid(InWidget))
		return;

	UFunction* functionResolved = InWidget->FindFunction(InFunctionName);
	if (functionResolved == nullptr)
	{
		CLog::Log(FString::Printf(TEXT("[SystemMessage] %s 미존재 — %s"),
			*InFunctionName.ToString(),
			*InWidget->GetClass()->GetName()));
		return;
	}

	InWidget->ProcessEvent(functionResolved, nullptr);
}

void UCSystemMessageComponent::ClearScheduledHideTimer()
{
	UWorld* worldResolved = GetWorld();
	if (false == IsValid(worldResolved))
		return;

	if (HideTimerHandle.IsValid())
		worldResolved->GetTimerManager().ClearTimer(HideTimerHandle);
}
