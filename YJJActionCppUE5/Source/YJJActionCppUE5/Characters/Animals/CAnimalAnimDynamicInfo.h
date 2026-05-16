#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAnimalAnimDynamicInfo.generated.h"

class APawn;

// ABP_AnimalAnim_DynamicInfo 레거시 — AnimInstance(말 등)가 스폰해 매 틱 갱신 값을 넘기는 보조 액터.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACAnimalAnimDynamicInfo : public AActor
{
	GENERATED_BODY()

public:
	ACAnimalAnimDynamicInfo();

	// ABP Tick_Info — 그래프는 BP 서브클래스에서 구현하거나 C++ 에서 오버라이드.
	UFUNCTION(BlueprintNativeEvent, Category = "AnimalAnim")
	void Tick_Info(APawn* InOwner);
	virtual void Tick_Info_Implementation(APawn* InOwner);
};
