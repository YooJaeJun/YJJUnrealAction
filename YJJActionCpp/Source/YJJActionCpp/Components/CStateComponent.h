#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CStateComponent.generated.h"

class ACCommonCharacter;

UENUM(BlueprintType)
enum class EStateType : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Fall		UMETA(DisplayName = "Fall"),
	Avoid		UMETA(DisplayName = "Avoid"),
	Equip		UMETA(DisplayName = "Equip"),
	Act			UMETA(DisplayName = "Act"),
	Hit			UMETA(DisplayName = "Hit"),
	Dead		UMETA(DisplayName = "Dead"),
	Rise		UMETA(DisplayName = "Rise"),
	Ride		UMETA(DisplayName = "Ride"),
	Max			UMETA(DisplayName = "Max")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStateTypeChanged, const EStateType, InPrevType, const EStateType, InNewType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCStateComponent();

protected:
	virtual void BeginPlay() override;

public:
	void SetIdleMode();
	void SetFallMode();
	void SetAvoidMode();
	void SetEquipMode();
	void SetActMode();
	void SetHitMode();
	void SetDeadMode();
	void SetRiseMode();
	void SetRideMode();

private:
	void ChangeType(const EStateType InType);

public:
	FORCEINLINE EStateType GetCurMode() const { return CurMode; }
	FORCEINLINE EStateType GetPrevMode() const { return PrevType; }
	FORCEINLINE bool IsIdleMode() const { return CurMode == EStateType::Idle; }
	FORCEINLINE bool IsFallMode() const { return CurMode == EStateType::Fall; }
	FORCEINLINE bool IsAvoidMode() const { return CurMode == EStateType::Avoid; }
	FORCEINLINE bool IsEquipMode() const { return CurMode == EStateType::Equip; }
	FORCEINLINE bool IsActMode() const { return CurMode == EStateType::Act; }
	FORCEINLINE bool IsHitMode() const { return CurMode == EStateType::Hit; }
	FORCEINLINE bool IsDeadMode() const { return CurMode == EStateType::Dead; }
	FORCEINLINE bool IsRiseMode() const { return CurMode == EStateType::Rise; }
	FORCEINLINE bool IsRideMode() const { return CurMode == EStateType::Ride; }

public:
	UPROPERTY(EditAnyWhere, Category = "Settings")
		EStateType CurMode = EStateType::Idle;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		EStateType PrevType = EStateType::Max;

public:
	FStateTypeChanged OnStateTypeChanged;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
};
