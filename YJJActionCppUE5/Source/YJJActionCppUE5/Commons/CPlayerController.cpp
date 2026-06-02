#include "Commons/CPlayerController.h"
#include "Buildings/CPlacedActor.h"
#include "Blueprint/UserWidget.h"
#include "Global.h"
#include "Components/CCharacterStatComponent.h"
#include "Components/CInventoryComponent.h"
#include "Commons/CGameState.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Items/CWorldItemActor.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Widgets/Player/CUserWidget_PlayerInfo.h"
#include "Characters/Player/CPlayableCharacter.h"

namespace
{
	void ResolvePlayerHudWidgetClass(TSubclassOf<UCUserWidget_HUD>& OutClass)
	{
		if (IsValid(OutClass))
			return;

		static const TCHAR* const HudWidgetPaths[] = {
			TEXT("/Game/Widgets/CWB_HUD.CWB_HUD_C"),
			TEXT("/Game/Widgets/WB_HUDUI.WB_HUDUI_C"),
			TEXT("/Game/Widgets/HUD.HUD_C"),
		};

		for (const TCHAR* const widgetPath : HudWidgetPaths)
		{
			YJJHelpers::GetClassDynamic<UCUserWidget_HUD>(&OutClass, widgetPath);
			if (IsValid(OutClass))
				return;
		}
	}
}

ACPlayerController::ACPlayerController()
{
	bReplicates = true;
	DefaultPlacedActorClass = ACPlacedActor::StaticClass();
	// HUD 블루프린트는 BeginPlay/EnsureHUD 시점에만 로드한다.
	// ctor 에서 StaticLoadClass 하면 CDO 생성·모듈 기동 중 위젯 BP 컴파일이 돌며
	// WidgetVariableNameToGuidMap 정리 Ensure 가 에디터 기동 직후에 걸릴 수 있다.
}

void ACPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
		ScheduleInitializeHUDForPawn();
}

void ACPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	if (IsLocalController())
		ScheduleInitializeHUDForPawn();
}

void ACPlayerController::ScheduleInitializeHUDForPawn()
{
	UWorld* World = GetWorld();
	if (false == IsValid(World))
		return;

	// OnRep_ReplicatedHasBegunPlay 등 AsyncLoading 중 StaticLoadClass 는 위젯 BP 동기 컴파일·Ensure 를 유발한다.
	if (IsAsyncLoading())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &ACPlayerController::ScheduleInitializeHUDForPawn);
		return;
	}

	InitializeHUDForPawn(GetPawn());
}

UCUserWidget_HUD* ACPlayerController::EnsureHUD()
{
	if (false == IsLocalController())
	{
		CLog::Log(FString::Printf(TEXT("[UI] EnsureHUD: 로컬 PlayerController 가 아님 — %s"), *GetName()));
		return nullptr;
	}

	ResolvePlayerHudWidgetClass(PlayerHUDClass);

	if (false == IsValid(PlayerHUD) && IsValid(PlayerHUDClass))
	{
		PlayerHUD = CreateWidget<UCUserWidget_HUD>(this, PlayerHUDClass);
		if (IsValid(PlayerHUD))
		{
			// Viewport 등록은 HUD 루트 하나만 담당한다. 하위 UI는 HUD 내부 위젯으로만 관리한다.
			PlayerHUD->AddToViewport();
		}
		else
			CLog::Log(FString::Printf(
				TEXT("[UI] EnsureHUD: HUD 위젯 생성 실패 — %s (UMG 에서 Compile & Save 로 stale GUID 정리)"),
				*PlayerHUDClass->GetPathName()));
	}
	else if (false == IsValid(PlayerHUD) && false == IsValid(PlayerHUDClass))
		CLog::Log(TEXT("[UI] EnsureHUD: PlayerHUDClass 로드 실패 — CWB_HUD/WB_HUDUI 부모=UCUserWidget_HUD(YJJActionCppUE5) 후 Compile & Save."));

	return PlayerHUD;
}

UCUserWidget_HUD* ACPlayerController::GetYJJHUD() const
{
	return PlayerHUD;
}

UCUserWidget_HUD* ACPlayerController::GetWBHUDUI()
{
	return EnsureHUD();
}

void ACPlayerController::InitializeHUDForPawn(APawn* InPawn)
{
	UCUserWidget_HUD* hud = EnsureHUD();
	if (false == IsValid(hud) || false == IsValid(InPawn))
	{
		CLog::Log(FString::Printf(TEXT("[UI] InitializeHUDForPawn: HUD 또는 Pawn 무효 — HUD=%s Pawn=%s"),
			IsValid(hud) ? TEXT("유효") : TEXT("무효"),
			IsValid(InPawn) ? *InPawn->GetName() : TEXT("(없음)")));
		return;
	}

	hud->SetChildren();

	UCCharacterStatComponent* characterStatComp = InPawn->FindComponentByClass<UCCharacterStatComponent>();
	if (IsValid(characterStatComp) && IsValid(hud->GetPlayerInfoWidget()))
		hud->GetPlayerInfoWidget()->BindStats(characterStatComp);

	ACPlayableCharacter* const playable = Cast<ACPlayableCharacter>(InPawn);
	if (IsValid(playable))
	{
		playable->BindLocalPlayerUI();
	}
}

void ACPlayerController::RequestPickup(ACWorldItemActor* WorldItem)
{
	if (HasAuthority())
	{
		Server_RequestPickup_Implementation(WorldItem);

		return;
	}

	Server_RequestPickup(WorldItem);
}

void ACPlayerController::RequestPlacement(const FTransform& RequestedTransform, const FName ItemID)
{
	if (HasAuthority())
	{
		Server_RequestPlacement_Implementation(RequestedTransform, ItemID);

		return;
	}

	Server_RequestPlacement(RequestedTransform, ItemID);
}

void ACPlayerController::Server_RequestPickup_Implementation(ACWorldItemActor* WorldItem)
{
	if (false == IsValid(WorldItem))
	{
		CLog::Log(FString::Printf(TEXT("[Dedicated] Server_RequestPickup: WorldItem 무효 — PC=%s"), *GetName()));
		Client_NotifyServerActionResult(false, FName(TEXT("InvalidItem")));

		return;
	}

	const bool bSucceeded = WorldItem->TryPickup(this);
	Client_NotifyServerActionResult(bSucceeded, bSucceeded ? FName(TEXT("PickupSucceeded")) : FName(TEXT("PickupRejected")));
}

void ACPlayerController::Server_RequestPlacement_Implementation(FTransform RequestedTransform, FName ItemID)
{
	if (false == ValidatePlacementRequest(RequestedTransform, ItemID))
	{
		Client_NotifyServerActionResult(false, FName(TEXT("PlacementRejected")));

		return;
	}

	UCInventoryComponent* inventoryComp = FindInventoryComponent();
	if (false == IsValid(inventoryComp) || false == inventoryComp->RemoveItem(ItemID, 1))
	{
		CLog::Log(FString::Printf(TEXT("[Dedicated] Server_RequestPlacement: 인벤토리 없음 또는 RemoveItem 실패 — ItemID=%s PC=%s"),
			*ItemID.ToString(), *GetName()));
		Client_NotifyServerActionResult(false, FName(TEXT("MissingItem")));

		return;
	}

	// 스폰 실패 시 복구할 수 있도록, 차감 이후의 실패 경로는 아이템을 되돌린다.
	UWorld* world = GetWorld();
	if (false == IsValid(world))
	{
		CLog::Log(TEXT("[Dedicated] Server_RequestPlacement: World 무효, 인벤토리 롤백"));
		inventoryComp->AddItem(ItemID, 1);
		Client_NotifyServerActionResult(false, FName(TEXT("InvalidWorld")));

		return;
	}

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = GetPawn();
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	ACPlacedActor* placedActor = world->SpawnActor<ACPlacedActor>(DefaultPlacedActorClass, RequestedTransform, spawnParams);
	if (false == IsValid(placedActor))
	{
		CLog::Log(FString::Printf(TEXT("[Dedicated] Server_RequestPlacement: PlacedActor 스폰 실패 — ItemID=%s"), *ItemID.ToString()));
		inventoryComp->AddItem(ItemID, 1);
		Client_NotifyServerActionResult(false, FName(TEXT("SpawnFailed")));

		return;
	}

	placedActor->InitializePlacedActor(ItemID);
	placedActor->EnterPlacedDormancy();

	ACGameState* gameState = world->GetGameState<ACGameState>();
	if (IsValid(gameState))
		gameState->AddPlacedActorCount(1);

	Client_NotifyServerActionResult(true, FName(TEXT("PlacementSucceeded")));
}

void ACPlayerController::Client_NotifyServerActionResult_Implementation(bool bSucceeded, FName Reason)
{
	UE_LOG(LogTemp, Log, TEXT("Server action result: %s (%s)"), bSucceeded ? TEXT("Success") : TEXT("Failed"), *Reason.ToString());
}

bool ACPlayerController::ValidatePlacementRequest(const FTransform& RequestedTransform, const FName ItemID) const
{
	if (ItemID == NAME_None || false == IsValid(DefaultPlacedActorClass))
		return false;

	const APawn* controlledPawn = GetPawn();
	if (false == IsValid(controlledPawn))
		return false;

	const float distanceSquared = FVector::DistSquared(controlledPawn->GetActorLocation(), RequestedTransform.GetLocation());
	if (distanceSquared > FMath::Square(MaxPlacementDistance))
		return false;

	UWorld* world = GetWorld();
	if (false == IsValid(world))
		return false;

	// 클라 프리뷰 결과를 그대로 믿지 않고 서버에서 거리와 충돌을 다시 판정한다.
	const FCollisionShape collisionShape = FCollisionShape::MakeSphere(PlacementCollisionRadius);
	const bool bBlocked = world->OverlapBlockingTestByChannel(
		RequestedTransform.GetLocation(),
		RequestedTransform.GetRotation(),
		ECC_WorldStatic,
		collisionShape);

	if (bBlocked)
		return false;

	UCInventoryComponent* inventoryComp = FindInventoryComponent();

	return IsValid(inventoryComp) && inventoryComp->HasItem(ItemID, 1);
}

UCInventoryComponent* ACPlayerController::FindInventoryComponent() const
{
	const APawn* controlledPawn = GetPawn();
	if (IsValid(controlledPawn))
	{
		UCInventoryComponent* pawnInventoryComp = controlledPawn->FindComponentByClass<UCInventoryComponent>();
		if (IsValid(pawnInventoryComp))
			return pawnInventoryComp;
	}

	if (IsValid(PlayerState))
		return PlayerState->FindComponentByClass<UCInventoryComponent>();

	return nullptr;
}
