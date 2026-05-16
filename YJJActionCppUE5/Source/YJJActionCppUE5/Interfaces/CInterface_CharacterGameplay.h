#pragma once

// 구 BP I_Character 의 세팅/이벤트 일부 — Land·TogglePossess 등은 다른 네이티브 인터페이스 또는 캐릭터 멤버로 이미 존재.
// 레거시 BPI 시그니처와 맞지 않으면 BP_Character 컴파일 시 ConformImplementedEvents → FixOverriddenEventSignature Ensure 가 날 수 있다.

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Chaos/ChaosEngineInterface.h"

class AActor;

#include "CInterface_CharacterGameplay.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UCInterface_CharacterGameplay : public UInterface
{
	GENERATED_BODY()
};
class YJJACTIONCPPUE5_API ICInterface_CharacterGameplay
{
	GENERATED_BODY()

public:
	// 구 I_Character 의 Footstep 발자국 이벤트(대개 bool + Surface + 위치). 이전 파라미터 없는 시그니처는 애님에서 FootstepAt 을 쓰던 케이스가 많다.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "세팅")
	void Footstep(
		bool bLeftFoot = false,
		EPhysicalSurface SurfaceType = SurfaceType_Default,
		FVector HitLocation = FVector::ZeroVector);

	// 구 I_Reward — 레거시는 보통 (호출 스폰, Exp, BuffIndex) 순서 또는 Invoker+Pawn 등이 많다 — Invoker 미사용 호출 호환 위해 nullptr 기본값.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "세팅")
	void Rewarded(AActor* RewardInvoker = nullptr, double Exp = 0.0, int32 BuffIndex = 0);

	// 구 I_Damage — 레거시 BPI 의 Damaged(float) 와 블루프린트에서 동일 표시명으로 남으면 UFunction 포인터는 달라도 이름이 겹쳐 ConformImplementedEvents → FixOverriddenEventSignature ensure 가 난다.
	// 네이티브 쪽만 다른 심볼명(GameplayDamaged)으로 분리한다. DisplayName 은 그래프에서 구분용.
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "세팅",
		meta = (DisplayName = "Damaged (Character Gameplay)"))
	void GameplayDamaged(float DamageAmount);

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
