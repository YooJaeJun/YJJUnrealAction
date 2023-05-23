#pragma once
#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_Interaction.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class YJJACTIONCPP_API UCUserWidget_Interaction : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void SetChild();

public:
	UPROPERTY()
		UImage* KeyIcon;

	UPROPERTY()
		UTextBlock* InteractionText;
};
