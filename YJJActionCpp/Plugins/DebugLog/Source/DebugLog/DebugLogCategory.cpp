#include "DebugLogCategory.h"
#include "DrawDebugHelpers.h"
#include "CanvasItem.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Components/CFeetComponent.h"

FDebugLogCategory::FDebugLogCategory()
{
	bShowOnlyWithDebugActor = false;
}

FDebugLogCategory::~FDebugLogCategory()
{
}

TSharedRef<FGameplayDebuggerCategory> FDebugLogCategory::MakeInstance()
{
	return MakeShareable(new FDebugLogCategory());
}

void FDebugLogCategory::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	FGameplayDebuggerCategory::CollectData(OwnerPC, DebugActor);

	const TWeakObjectPtr<ACPlayableCharacter> player = OwnerPC->GetPawn<ACPlayableCharacter>();
	if (false == player.IsValid())
		return;

	// Player
	{
		PlayerData.bDraw = true;
		PlayerData.Name = player->GetName();
		PlayerData.Location = player->GetActorLocation();
		PlayerData.Rotation = player->GetActorRotation();

		const TWeakObjectPtr<UCFeetComponent> feetComp = 
			Cast<UCFeetComponent>(player->GetComponentByClass(UCFeetComponent::StaticClass()));

		PlayerData.FeetData.LeftDistance = feetComp->GetData().LeftDistance;
		PlayerData.FeetData.RightDistance = feetComp->GetData().RightDistance;
		PlayerData.FeetData.LeftRotation = feetComp->GetData().LeftRotation;
		PlayerData.FeetData.RightRotation = feetComp->GetData().RightRotation;
		PlayerData.FeetData.PelvisDistance = feetComp->GetData().PelvisDistance;
	}
}

void FDebugLogCategory::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	FGameplayDebuggerCategory::DrawData(OwnerPC, CanvasContext);

	// Box
	FCanvasTileItem item(
		FVector2D(10, 10),
		FVector2D(320, 215),
		FLinearColor(0, 0, 0, 0.25f));
	item.BlendMode = ESimpleElementBlendMode::SE_BLEND_AlphaBlend;

	CanvasContext.DrawItem(item, CanvasContext.CursorX, CanvasContext.CursorY);

	// Player
	if (true == PlayerData.bDraw)
	{
		CanvasContext.Printf(FColor::Green, L" Player");
		CanvasContext.Printf(FColor::White, L" Info---------------------------------------------");
		CanvasContext.Printf(FColor::White, L"  Name : %s", *PlayerData.Name);
		CanvasContext.Printf(FColor::White, L"  Location : %s", *PlayerData.Location.ToString());
		CanvasContext.Printf(FColor::White, L"  Rotation : %s", *PlayerData.Rotation.ToString());
		CanvasContext.Printf(FColor::White, L" IK-----------------------------------------------");
		CanvasContext.Printf(FColor::White, L"  LeftDistance : %s", *PlayerData.FeetData.LeftDistance.ToString());
		CanvasContext.Printf(FColor::White, L"  RightDistance : %s", *PlayerData.FeetData.RightDistance.ToString());
		CanvasContext.Printf(FColor::White, L"  PelvisDistance : %s", *PlayerData.FeetData.PelvisDistance.ToString());
		CanvasContext.Printf(FColor::White, L"  LeftRotation : %s", *PlayerData.FeetData.LeftRotation.ToString());
		CanvasContext.Printf(FColor::White, L"  RightRotation : %s", *PlayerData.FeetData.RightRotation.ToString());
	}
}