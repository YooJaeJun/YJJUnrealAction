#include "Widgets/Interaction/CUserWidget_Interaction.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UCUserWidget_Interaction::SetChild()
{
	KeyIcon = Cast<UImage>(GetWidgetFromName(TEXT("CKeyIcon")));
	InteractionText = Cast<UTextBlock>(GetWidgetFromName(TEXT("CInteractionText")));
}
