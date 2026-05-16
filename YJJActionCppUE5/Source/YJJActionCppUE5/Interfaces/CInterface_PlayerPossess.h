#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CInterface_PlayerPossess.generated.h"

// 구 BP 인터페이스 I_PlayerPossess::TogglePossess 대체. BP 에서 Implement Interface 로 붙인다.
UINTERFACE(MinimalAPI, BlueprintType)
class UCInterface_PlayerPossess : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPPUE5_API ICInterface_PlayerPossess
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "탑승", meta = (DisplayName = "Toggle Possess"))
	void TogglePossess(bool InEnable);
};
