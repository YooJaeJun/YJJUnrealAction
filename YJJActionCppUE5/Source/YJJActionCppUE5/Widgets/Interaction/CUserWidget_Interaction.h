#pragma once
#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_Interaction.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class YJJACTIONCPPUE5_API UCUserWidget_Interaction : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	void BindChildren();
	void SetChildren(TObjectPtr<UTexture2D> InKeyTexture, const FText& InText);

private:
	void SetKeyIcon(TObjectPtr<UTexture2D> InKeyTexture);
	void SetText(const FText& InText);

public:
	UPROPERTY()
		TObjectPtr<UImage> KeyIcon;

	UPROPERTY()
		TObjectPtr<UTextBlock> Text;
};
