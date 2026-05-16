#pragma once
#include "CoreMinimal.h"
#include "CollisionQueryParams.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "CTargetingComponent.generated.h"

class ACCommonCharacter;
class AController;
class UCStateComponent;
class UCMovementComponent;
class UWidgetComponent;
class UParticleSystem;
class UParticleSystemComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YJJACTIONCPPUE5_API UCTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCTargetingComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION()
	void InputAction_Targeting();

	void Begin_Targeting();
	void End_Targeting();
	void ChangeTarget(ACCommonCharacter* InTarget);
	void SetVisibleTargetUI(bool bVisible);
	void Tick_MoveFocusCoolTIme(const float InDelta);
	void Tick_Targeting();

	// 레거시 BP TargetComponent — 블루프린트 줌/포커스 그래프에서 호출.
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void ChangeFocus(bool InRight);

	// 레거시 BP TargetComponent::Toggle_Target
	UFUNCTION(BlueprintCallable, Category = "Targeting", meta = (DisplayName = "Toggle Target"))
	void Toggle_Target() { InputAction_Targeting(); }

	UFUNCTION(BlueprintPure, Category = "Targeting")
	bool IsTargeting() const { return bTargeting; }

public:
	// BP `IsTargeting` — 대상 고정 상태(블루프린트 디폴트 false).
	UPROPERTY(VisibleAnywhere, Category = "MovingFocus")
	bool bTargeting = false;

	// BP `Moving Focus`
	UPROPERTY(VisibleAnywhere, Category = "MovingFocus")
	bool bMovingFocus = false;

	// BP `Can Move Focus` (디폴트 true 그래프가 기대함).
	UPROPERTY(VisibleAnywhere, Category = "MovingFocus")
	bool bCanMoveFocus = true;

	// BP 내부용 `MovingFocusCurCoolTime`.
	UPROPERTY(VisibleAnywhere, Category = "MovingFocus")
	float MovingFocusCurCoolTime = 0.0f;

	// BP `Moving Focus Init Time` (0.3).
	UPROPERTY(VisibleAnywhere, Category = "MovingFocus")
	float MovingFocusInitTime = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TraceSetting")
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebug = EDrawDebugTrace::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TraceSetting")
	float TraceDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TraceSetting")
	float FinishAngle = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TraceSetting")
	float InterpSpeed = 20.0f;

	// 레거시 캐스케이드 파티클(에디터에서만 붙이는 경우 많음 — 데이터만 BP 와 동일 카테고리로 유지).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TraceSetting")
	TObjectPtr<UParticleSystemComponent> Particle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TraceSetting")
	TObjectPtr<UParticleSystem> ParticleAsset;

private:
	// 블루프린트 `Character` 변수(BP_Character 캐시)에 대응.
	TWeakObjectPtr<ACCommonCharacter> OwnerCharacter;
	TWeakObjectPtr<ACCommonCharacter> Target;

	// 블루프린트 `Controller` 변수 — `BeginPlay` 와 타깃 틱에서 갱신.
	TWeakObjectPtr<AController> Controller;

	TWeakObjectPtr<UCStateComponent> TargetStateComp;
	TWeakObjectPtr<UCMovementComponent> TargetMovementComp;
	TWeakObjectPtr<UWidgetComponent> TargetingWidgetComp;

	// 레거시 BP 의 FixCharacterCamera / UnFixCharacterCamera — `UCMovementComponent` 경유로 소유 플레이어 카메라 고정.
	void FixCharacterCamera();
	void UnFixCharacterCamera();

	// 레거시 BP 마커 파티클 — `SpawnEmitterAttached` 로 부착, 종료 시 `DestroyComponent`.
	void DestroyMarkerParticle();
	void SpawnMarkerParticleForTarget(ACCommonCharacter* InTarget);
};
