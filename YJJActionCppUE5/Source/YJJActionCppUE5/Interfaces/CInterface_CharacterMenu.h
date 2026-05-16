#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CInterface_CharacterMenu.generated.h"

// 구 BP 인터페이스 I_Character 의 장비/마법 메뉴 이벤트 대체.
UINTERFACE(MinimalAPI, BlueprintType)
class UCInterface_CharacterMenu : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPPUE5_API ICInterface_CharacterMenu
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI|Menu")
	void HoveredEquipMenu(const FString& InName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI|Menu")
	void UnHoveredEquipMenu(const FString& InName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI|Menu")
	void ClickedEquipMenu(const FString& InName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI|Menu")
	void HoveredMagicMenu(const FString& InName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI|Menu")
	void UnHoveredMagicMenu(const FString& InName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI|Menu")
	void ClickedMagicMenu(const FString& InName);
};
