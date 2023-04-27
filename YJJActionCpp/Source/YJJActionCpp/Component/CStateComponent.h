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

	FORCEINLINE bool IsIdleMode() { return Type == EStateType::Idle; }
	FORCEINLINE bool IsEquipMode() { return Type == EStateType::Equip; }
	FORCEINLINE bool IsActMode() { return Type == EStateType::Act; }
	FORCEINLINE bool IsHitMode() { return Type == EStateType::Hit; }

public:
	UCStateComponent();

protected:
	virtual void BeginPlay() override;

public:
	void SetIdleMode();
	void SetEquipMode();
	void SetActMode();
	void SetHitMode();

private:
	void ChangedType(EStateType InType);

public:
	FStateTypeChanged OnStateTypeChanged;
};
