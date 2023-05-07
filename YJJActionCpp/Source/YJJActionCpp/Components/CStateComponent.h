#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CStateComponent.generated.h"

class ACCommonCharacter;

UENUM(BlueprintType)
enum class EStateType : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Avoid		UMETA(DisplayName = "Avoid"),
	Dash		UMETA(DisplayName = "Dash"),
	Fly			UMETA(DisplayName = "Fly"),
	Fall		UMETA(DisplayName = "Fall"),
	Equip		UMETA(DisplayName = "Equip"),
	Act			UMETA(DisplayName = "Act"),
	Hit			UMETA(DisplayName = "Hit"),
	Dead		UMETA(DisplayName = "Dead"),
	HitAir		UMETA(DisplayName = "HitAir"),
	DownFly		UMETA(DisplayName = "DownFly"),
	DownFall	UMETA(DisplayName = "DownFall"),
	Rise		UMETA(DisplayName = "Rise"),
	Ride		UMETA(DisplayName = "Ride"),
	RideAndAct	UMETA(DisplayName = "RideAndAct"),
	Cinematic	UMETA(DisplayName = "Cinematic"),
	Groggy		UMETA(DisplayName = "Groggy"),
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

private:
	void ChangeType(const EStateType InType);

public:
	FORCEINLINE bool IsIdleMode() const { return Type == EStateType::Idle; }
	FORCEINLINE bool IsFallMode() const { return Type == EStateType::Fall; }
	FORCEINLINE bool IsAvoidMode() const { return Type == EStateType::Avoid; }
	FORCEINLINE bool IsEquipMode() const { return Type == EStateType::Equip; }
	FORCEINLINE bool IsActMode() const { return Type == EStateType::Act; }
	FORCEINLINE bool IsHitMode() const { return Type == EStateType::Hit; }

public:
	UPROPERTY(EditAnyWhere, Category = "Settings")
		EStateType Type = EStateType::Idle;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		EStateType PrevType = EStateType::Max;

public:
	FStateTypeChanged OnStateTypeChanged;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
};
