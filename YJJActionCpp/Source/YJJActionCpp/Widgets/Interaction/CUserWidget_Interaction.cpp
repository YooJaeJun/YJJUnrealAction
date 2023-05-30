#include "Widgets/Interaction/CUserWidget_Interaction.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UCUserWidget_Interaction::BindChildren()
{
	KeyIcon = Cast<UImage>(GetWidgetFromName(TEXT("CKeyIcon")));
	Text = Cast<UTextBlock>(GetWidgetFromName(TEXT("CInteractionText")));
}

void UCUserWidget_Interaction::SetChildren(UTexture2D* InKeyTexture, const FText& InText)
{
	SetKeyIcon(InKeyTexture);
	SetText(InText);
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCUserWidget_Interaction::SetKeyIcon(UTexture2D* InKeyTexture)
{
	KeyIcon->SetBrushFromTexture(InKeyTexture);
}

void UCUserWidget_Interaction::SetText(const FText& InText)
{
	Text->SetText(InText);
}
