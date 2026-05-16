#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CInterface_Interactable.generated.h"

class ACCommonCharacter;

UINTERFACE(MinimalAPI)
class UCInterface_Interactable : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPPUE5_API ICInterface_Interactable
{
	GENERATED_BODY()

public:
	virtual void Interact(TObjectPtr<ACCommonCharacter> InteractingActor) {}

	// BP I_Interactable::GetAction — 상호작용 프롬프트(빈 문자열이면 UI 에서 숨기거나 기본값).
	virtual void GetAction(FText& OutText) const { OutText = FText::GetEmpty(); }

	// BP I_Interactable::EndInteraction — 탑승 해제 등 상호작용 종료. 기본은 무처리.
	virtual void EndInteraction() {}
};
