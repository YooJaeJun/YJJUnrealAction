#pragma once
#include "CoreMinimal.h"
#include "Characters/CAnimInstance_Character.h"
#include "CAnimInstance_Animal.generated.h"

class ACAnimalAnimDynamicInfo;

// ABP_Lion / ABP_Zebra / ABP_Wolf / ABP_Horse 등 공통 — Info 액터 스폰·Tick_Info 는 C++ 고정.
UCLASS()
class YJJACTIONCPPUE5_API UCAnimInstance_Animal : public UCAnimInstance_Character
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUninitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	void EnsureInfoActor();

	// BPVar Info / InfoClass — 레거시 ABP Reparent 후 동일 카테고리·표시명 유지.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animal", meta = (DisplayName = "Info Class"))
	TSubclassOf<ACAnimalAnimDynamicInfo> InfoClass;

	// 레거시 ABP 가 "Set Info" 등으로 변수를 채운다 — ReadOnly 로 두면 에디터 재컴파일이 막히므로 쓰기 허용.
	// C++ 에서 Spawn 한 경우 Destroy 는 bSpawnedInfo 기준으로 NativeUninitializeAnimation 에서만 수행한다.
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Animal", meta = (DisplayName = "Info"))
	TObjectPtr<ACAnimalAnimDynamicInfo> Info;

	// ACCommonCharacter::bIsRotating 과 매 틱 동기화 (ABP_Dragon 등 AnimInstance 로컬 bool 호환).
	UPROPERTY(BlueprintReadOnly, Category = "Animal", meta = (DisplayName = "Is Rotating"))
	bool bRotating = false;

private:
	bool bSpawnedInfo = false;
};
