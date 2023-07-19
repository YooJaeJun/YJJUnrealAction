#pragma once
#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_CrossHair.generated.h"

UCLASS()
class YJJACTIONCPP_API UCUserWidget_CrossHair : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
		void SetColor_Red();
	void SetColor_Red_Implementation() {}

	UFUNCTION(BlueprintImplementableEvent)
		void SetColor_White();
	void SetColor_White_Implementation() {}
};
