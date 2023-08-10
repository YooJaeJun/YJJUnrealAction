#pragma once

#include "CoreMinimal.h"
#include "GameplayDebuggerCategory.h"
#include "Components/CFeetComponent.h"

class DEBUGLOG_API FDebugLogCategory : public FGameplayDebuggerCategory
{
public:
	FDebugLogCategory();
	virtual ~FDebugLogCategory() override;

public:
	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

public:
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

private:
	struct FCategoryData
	{
		bool bDraw = false;
		FString Name;
		FVector Location;
		FRotator Rotation;
		FFeetData FeetData;
	};

private:
	FCategoryData PlayerData;
};
