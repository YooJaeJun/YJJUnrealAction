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

	// 레거시 BP StateComponent::SetIdle 노드 이름 호환.
	UFUNCTION(BlueprintCallable, Category = "State", meta = (DisplayName = "Set Idle"))
	void SetIdle() { SetIdleMode(); }

	void SetFallMode();

	// BP_Player 등에서 StateComponent 대상으로 호출하던 Set Falling — Fall 모드로 전환.
	UFUNCTION(BlueprintCallable, Category = "State", meta = (DisplayName = "Set Falling"))
	void SetFalling() { SetFallMode(); }

	void SetAvoidMode();
	void SetEquipMode();
	void SetActMode();
	void SetDeadMode();

	// 레거시 BP — StateComponent 타깃 "Set Dead".
	UFUNCTION(BlueprintCallable, Category = "State", meta = (DisplayName = "Set Dead"))
	void SetDead() { SetDeadMode(); }

	void SetRiseMode();

	// 레거시 BP — "Set Rise".
	UFUNCTION(BlueprintCallable, Category = "State", meta = (DisplayName = "Set Rise"))
	void SetRise() { SetRiseMode(); }
	void SetRidingMode();

	// 레거시 BP — StateComponent 타깃 "Set Riding".
	UFUNCTION(BlueprintCallable, Category = "State", meta = (DisplayName = "Set Riding"))
	void SetRiding() { SetRidingMode(); }

	void SetHitMode(const CEHitType InHitType);

	// 레거시 BP_Player — "Set Hitted". 우선 공통 피격 모드로 둔다.
	UFUNCTION(BlueprintCallable, Category = "State", meta = (DisplayName = "Set Hitted"))
	void SetHitted() { SetHitCommonMode(); }
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

	// BP StateComponent::PrevHitType / 이전 피격 타입 조회.
	FORCEINLINE constexpr CEHitType GetPrevHitType() const { return PrevHitType; }

	FORCEINLINE constexpr bool IsIdleMode() const { return CurType == CEStateType::Idle; }

	// 레거시 BP — StateComponent 타깃 "Is Idle"(그래프가 IsIdleMode 대신 호출).
	UFUNCTION(BlueprintPure, Category = "State", meta = (DisplayName = "Is Idle"))
	bool IsIdle() const { return IsIdleMode(); }
	FORCEINLINE constexpr bool IsFallMode() const { return CurType == CEStateType::Fall; }
	FORCEINLINE constexpr bool IsAvoidMode() const { return CurType == CEStateType::Avoid; }
	FORCEINLINE constexpr bool IsEquipMode() const { return CurType == CEStateType::Equip; }
	FORCEINLINE constexpr bool IsActMode() const { return CurType == CEStateType::Act; }
	FORCEINLINE constexpr bool IsDeadMode() const { return CurType == CEStateType::Dead; }

	// 레거시 BP — StateComponent 타깃 "Is Dead"(전투 사망 상태).
	UFUNCTION(BlueprintPure, Category = "State")
	bool IsDead() const { return IsDeadMode(); }
	FORCEINLINE constexpr bool IsRiseMode() const { return CurType == CEStateType::Rise; }
	FORCEINLINE constexpr bool IsRidingMode() const { return CurType == CEStateType::Riding; }

	// 블루프린트 StateComponent::IsRealRiding — 실제 탑승 플래그는 캐릭터 쪽이다.
	UFUNCTION(BlueprintPure, Category = "State")
	bool IsRealRiding() const;

	// 레거시 BP_Player RestoreStamina/Mana — StateComponent 타깃 "Is Riding"(탑승 상태·플래그 둘 중 하나).
	UFUNCTION(BlueprintPure, Category = "State", meta = (DisplayName = "Is Riding"))
	bool IsRiding() const;

	FORCEINLINE constexpr bool IsHitNoneMode() const { return CurHitType == CEHitType::None; }
	FORCEINLINE constexpr bool IsHitCommonMode() const { return CurHitType == CEHitType::Common; }
	FORCEINLINE constexpr bool IsHitDownMode() const { return CurHitType == CEHitType::Down; }
	FORCEINLINE constexpr bool IsHitFlyMode() const { return CurHitType == CEHitType::Fly; }
	FORCEINLINE constexpr bool IsHitKnockbackMode() const { return CurHitType == CEHitType::Knockback; }
	FORCEINLINE constexpr bool IsHitAirHitMode() const { return CurHitType == CEHitType::Air; }
	FORCEINLINE constexpr bool IsHitFlyingPutDownHitMode() const { return CurHitType == CEHitType::FlyingPutDown; }

	// BP StateComponent::IsHitted — 피격 타입이 None 이 아닐 때 true.
	UFUNCTION(BlueprintPure, Category = "State")
	bool IsHitted() const { return false == IsHitNoneMode(); }

	// UHT: BlueprintReadOnly(Type) 는 private 에 둘 수 없음.
protected:
	UPROPERTY(EditAnyWhere, Category = "Settings")
	CEStateType CurType = CEStateType::Idle;

	// 레거시 BP StateComponent::Type 핀과 동일한 의미(CurType 미러). 브로드캐스트/검사용.
	UPROPERTY(BlueprintReadOnly, Category = "State")
	CEStateType Type = CEStateType::Idle;

	UPROPERTY(EditAnyWhere, Category = "Settings")
	CEStateType PrevType = CEStateType::Max;

	UPROPERTY(EditAnyWhere, Category = "Settings")
	CEHitType CurHitType = CEHitType::None;

	UPROPERTY(EditAnyWhere, Category = "Settings")
	CEHitType PrevHitType = CEHitType::Max;

public:
	UPROPERTY(BlueprintAssignable, Category = "State")
	FStateTypeChanged OnStateTypeChanged;

	UPROPERTY(BlueprintAssignable, Category = "State")
	FHitStateTypeChanged OnHitStateTypeChanged;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	bool bInSkillMode = false;
};