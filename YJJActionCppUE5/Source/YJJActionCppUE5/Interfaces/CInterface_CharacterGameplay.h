#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CInterface_CharacterGameplay.generated.h"

// 구 BP I_Character 의 세팅/이벤트 일부 — Land·TogglePossess 등은 다른 네이티브 인터페이스 또는 캐릭터 멤버로 이미 존재.
UINTERFACE(MinimalAPI, BlueprintType)
class UCInterface_CharacterGameplay : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPPUE5_API ICInterface_CharacterGameplay
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "세팅")
	void Footstep();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "세팅")
	void Rewarded();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "세팅")
	void Damaged(float DamageAmount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "세팅")
	void StartInteraction(AActor* InteractionTarget);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "세팅")
	void EndInteraction();

	// 이름을 GetCharacterGameplayAction 으로 둠 — 레거시 I_Interactable::GetAction(FText&) 와 블루프린트 인터페이스 목록에서 동명 충돌한다.
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "세팅",
		meta = (DisplayName = "Get Character Gameplay Action"))
	int32 GetCharacterGameplayAction();
};
