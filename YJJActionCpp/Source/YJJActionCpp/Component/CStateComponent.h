#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CStateComponent.generated.h"

UENUM(BlueprintType)
enum class EStateType : uint8
{
	Idle,
	Equip,
	Act,
	Hit,
	Dead,
	Fly,
	Fall,
	HitAir,
	DownFly,
	DownFall,
	Rise,
	BackStep,
	Dash,
	Ride,
	RideAndAct,
	Cinematic,
	Groggy,
	Max
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStateTypeChanged, EStateType, InPrevType, EStateType, InNewType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnyWhere, Category = "Settings")
		EStateType Type = EStateType::Max;

	FORCEINLINE bool IsIdleMode() const { return Type == EStateType::Idle; }
	FORCEINLINE bool IsEquipMode() const { return Type == EStateType::Equip; }
	FORCEINLINE bool IsActMode() const { return Type == EStateType::Act; }
	FORCEINLINE bool IsHitMode() const { return Type == EStateType::Hit; }
	FORCEINLINE bool IsJumpMode() const { return Type == EStateType::Fall; }

public:
	UCStateComponent();

protected:
	virtual void BeginPlay() override;

public:
	void SetIdleMode();
	void SetEquipMode();
	void SetActMode();
	void SetHitMode();
	void SetFallMode();

private:
	void ChangedType(EStateType InType);

public:
	FStateTypeChanged OnStateTypeChanged;
};
