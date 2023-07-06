#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Commons/CEnums.h"
#include "CStateComponent.generated.h"

class ACCommonCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStateTypeChanged, const EStateType, InPrevType, const EStateType, InNewType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHitStateTypeChanged, const EHitType, InPrevType, const EHitType, InNewType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCStateComponent();

protected:
	virtual void BeginPlay() override;

public:
	bool CanAttack() const;

public:
	FORCEINLINE void OnSkillMode() { bInSkillMode = true; }
	FORCEINLINE void OffSkillMode() { bInSkillMode = false; }

	void SetIdleMode();
	void SetFallMode();
	void SetAvoidMode();
	void SetEquipMode();
	void SetActMode();
	void SetDeadMode();
	void SetRiseMode();

	void SetHitNoneMode();
	void SetHitCommonMode();
	void SetHitDownMode();
	void SetHitFlyMode();
	void SetHitKnockbackMode();
	void SetHitAirMode();
	void SetHitFlyingPutDownMode();

private:
	void ChangeType(const EStateType InType);

private:
	void ChangeHitType(const EHitType InType);

public:
	FORCEINLINE constexpr bool IsSkillMode() const { return bInSkillMode; }

	FORCEINLINE constexpr EStateType GetCurMode() const { return CurType; }
	FORCEINLINE constexpr EStateType GetPrevMode() const { return PrevType; }
	FORCEINLINE constexpr EHitType GetCurHitMode() const { return CurHitType; }

	FORCEINLINE constexpr bool IsIdleMode() const { return CurType == EStateType::Idle; }
	FORCEINLINE constexpr bool IsFallMode() const { return CurType == EStateType::Fall; }
	FORCEINLINE constexpr bool IsAvoidMode() const { return CurType == EStateType::Avoid; }
	FORCEINLINE constexpr bool IsEquipMode() const { return CurType == EStateType::Equip; }
	FORCEINLINE constexpr bool IsActMode() const { return CurType == EStateType::Act; }
	FORCEINLINE constexpr bool IsDeadMode() const { return CurType == EStateType::Dead; }
	FORCEINLINE constexpr bool IsRiseMode() const { return CurType == EStateType::Rise; }

	FORCEINLINE constexpr bool IsHitNoneMode() const { return CurHitType == EHitType::None; }
	FORCEINLINE constexpr bool IsHitCommonMode() const { return CurHitType == EHitType::Common; }
	FORCEINLINE constexpr bool IsHitDownMode() const { return CurHitType == EHitType::Down; }
	FORCEINLINE constexpr bool IsHitFlyMode() const { return CurHitType == EHitType::Fly; }
	FORCEINLINE constexpr bool IsHitKnockbackMode() const { return CurHitType == EHitType::Knockback; }
	FORCEINLINE constexpr bool IsHitAirHitMode() const { return CurHitType == EHitType::Air; }
	FORCEINLINE constexpr bool IsHitFlyingPutDownHitMode() const { return CurHitType == EHitType::FlyingPutDown; }

private:
	UPROPERTY(EditAnyWhere, Category = "Settings")
		EStateType CurType = EStateType::Idle;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		EStateType PrevType = EStateType::Max;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		EHitType CurHitType = EHitType::None;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		EHitType PrevHitType = EHitType::Max;

public:
	FStateTypeChanged OnStateTypeChanged;
	FHitStateTypeChanged OnHitStateTypeChanged;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	bool bInSkillMode = false;
};