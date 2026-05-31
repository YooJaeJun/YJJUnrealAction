#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CUserWidget_Custom.generated.h"

class ACCommonCharacter;

UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API UCUserWidget_Custom : public UUserWidget
{
	GENERATED_BODY()

private:
	TSharedPtr<ACCommonCharacter> Owner;
};
