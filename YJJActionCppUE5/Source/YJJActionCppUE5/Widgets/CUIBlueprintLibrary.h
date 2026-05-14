#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CUIBlueprintLibrary.generated.h"

class UCUserWidget_HUD;

UCLASS()
class YJJACTIONCPPUE5_API UCUIBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI", meta = (WorldContext = "WorldContextObject"))
	static UCUserWidget_HUD* EnsureLocalHUD(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "UI", meta = (WorldContext = "WorldContextObject"))
	static UCUserWidget_HUD* GetLocalHUD(const UObject* WorldContextObject);
};
