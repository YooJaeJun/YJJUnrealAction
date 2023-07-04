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
	Land		UMETA(DisplayName = "Land"),
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
	FORCEINLINE void OnSkillMode() { bInSkillMode = true; }
	FORCEINLINE void OffSkillMode() { bInSkillMode = false; }

	bool CanAttack() const;
	void GoBack();

private:
	void ChangeType(const EStateType InType);

public:
	FORCEINLINE constexpr EStateType GetCurMode() const { return CurType; }
	FORCEINLINE constexpr EStateType GetPrevMode() const { return PrevType; }
	FORCEINLINE constexpr bool IsIdleMode() const { return CurType == EStateType::Idle; }
	FORCEINLINE constexpr bool IsFallMode() const { return CurType == EStateType::Fall; }
	FORCEINLINE constexpr bool IsAvoidMode() const { return CurType == EStateType::Avoid; }
	FORCEINLINE constexpr bool IsEquipMode() const { return CurType == EStateType::Equip; }
	FORCEINLINE constexpr bool IsActMode() const { return CurType == EStateType::Act; }
	FORCEINLINE constexpr bool IsHitMode() const { return CurType == EStateType::Hit; }
	FORCEINLINE constexpr bool IsDeadMode() const { return CurType == EStateType::Dead; }
	FORCEINLINE constexpr bool IsRiseMode() const { return CurType == EStateType::Rise; }
	FORCEINLINE constexpr bool IsSkillMode() const { return bInSkillMode; }

private:
	UPROPERTY(EditAnyWhere, Category = "Settings")
		EStateType CurType = EStateType::Idle;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		EStateType PrevType = EStateType::Max;

public:
	FStateTypeChanged OnStateTypeChanged;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	bool bInSkillMode = false;
};
