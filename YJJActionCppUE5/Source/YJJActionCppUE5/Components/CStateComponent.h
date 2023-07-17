#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Commons/CEnums.h"
#include "CStateComponent.generated.h"

class ACCommonCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStateTypeChanged, const CEStateType, InPrevType, const CEStateType, InNewType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHitStateTypeChanged, const CEHitType, InPrevType, const CEHitType, InNewType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPPUE5_API UCStateComponent : public UActorComponent
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

	void SetHitMode(const CEHitType InHitType);
	void SetHitNoneMode();
	void SetHitCommonMode();
	void SetHitDownMode();
	void SetHitFlyMode();
	void SetHitKnockbackMode();
	void SetHitAirMode();
	void SetHitFlyingPutDownMode();

private:
	void ChangeType(const CEStateType InType);

private:
	void ChangeHitType(const CEHitType InType);

public:
	FORCEINLINE constexpr bool IsSkillMode() const { return bInSkillMode; }

	FORCEINLINE constexpr CEStateType GetCurMode() const { return CurType; }
	FORCEINLINE constexpr CEStateType GetPrevMode() const { return PrevType; }
	FORCEINLINE constexpr CEHitType GetCurHitMode() const { return CurHitType; }

	FORCEINLINE constexpr bool IsIdleMode() const { return CurType == CEStateType::Idle; }
	FORCEINLINE constexpr bool IsFallMode() const { return CurType == CEStateType::Fall; }
	FORCEINLINE constexpr bool IsAvoidMode() const { return CurType == CEStateType::Avoid; }
	FORCEINLINE constexpr bool IsEquipMode() const { return CurType == CEStateType::Equip; }
	FORCEINLINE constexpr bool IsActMode() const { return CurType == CEStateType::Act; }
	FORCEINLINE constexpr bool IsDeadMode() const { return CurType == CEStateType::Dead; }
	FORCEINLINE constexpr bool IsRiseMode() const { return CurType == CEStateType::Rise; }

	FORCEINLINE constexpr bool IsHitNoneMode() const { return CurHitType == CEHitType::None; }
	FORCEINLINE constexpr bool IsHitCommonMode() const { return CurHitType == CEHitType::Common; }
	FORCEINLINE constexpr bool IsHitDownMode() const { return CurHitType == CEHitType::Down; }
	FORCEINLINE constexpr bool IsHitFlyMode() const { return CurHitType == CEHitType::Fly; }
	FORCEINLINE constexpr bool IsHitKnockbackMode() const { return CurHitType == CEHitType::Knockback; }
	FORCEINLINE constexpr bool IsHitAirHitMode() const { return CurHitType == CEHitType::Air; }
	FORCEINLINE constexpr bool IsHitFlyingPutDownHitMode() const { return CurHitType == CEHitType::FlyingPutDown; }

private:
	UPROPERTY(EditAnyWhere, Category = "Settings")
		CEStateType CurType = CEStateType::Idle;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		CEStateType PrevType = CEStateType::Max;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		CEHitType CurHitType = CEHitType::None;

	UPROPERTY(EditAnyWhere, Category = "Settings")
		CEHitType PrevHitType = CEHitType::Max;

public:
	FStateTypeChanged OnStateTypeChanged;
	FHitStateTypeChanged OnHitStateTypeChanged;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	bool bInSkillMode = false;
};