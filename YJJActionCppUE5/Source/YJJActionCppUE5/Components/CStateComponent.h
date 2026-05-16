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

	void SetHitNoneMode();
	// 레거시 BP_Player 피격 "타입" 공통(Common). StateComponent 블루프린트 SetHitted(ChangeType) 과 다르다.
	void SetHitCommonMode();
	void SetHitDownMode();
	void SetHitFlyMode();
	void SetHitKnockbackMode();
	// CEHitType::Air. 상태 전환 SetHitAir(이동 상태) 와 이름이 겹치지 않게 Reaction 으로 구분했다.
	void SetHitReactionAir();
	void SetHitFlyingPutDownMode();

	// 레거시 BP StateComponent 그래프: SetEquip/SetAction/SetHitted 등은 ChangeType 과 동등.
	UFUNCTION(BlueprintCallable, Category = "State|Mode", meta = (DisplayName = "Set Equip"))
	void SetEquip() { SetEquipMode(); }

	UFUNCTION(BlueprintCallable, Category = "State|Mode", meta = (DisplayName = "Set Action"))
	void SetAction() { SetActMode(); }

	UFUNCTION(BlueprintCallable, Category = "State|Mode", meta = (DisplayName = "Set Hitted"))
	void SetHitted() { InternalSetStateType(CEStateType::CombatHitted); }

	UFUNCTION(BlueprintCallable, Category = "State|Mode", meta = (DisplayName = "Set Parkour"))
	void SetParkour() { InternalSetStateType(CEStateType::Parkour); }

	UFUNCTION(BlueprintCallable, Category = "State|Mode", meta = (DisplayName = "Set Flying"))
	void SetFlying() { InternalSetStateType(CEStateType::Flying); }

	UFUNCTION(BlueprintCallable, Category = "State|Mode", meta = (DisplayName = "Set Hit Air"))
	void SetHitAir() { InternalSetStateType(CEStateType::HitAir); }

	UFUNCTION(BlueprintCallable, Category = "State|Mode", meta = (DisplayName = "Set Down Flying"))
	void SetDownFlying() { InternalSetStateType(CEStateType::DownFlying); }

	UFUNCTION(BlueprintCallable, Category = "State|Mode", meta = (DisplayName = "Set Down Land"))
	void SetDownLand() { InternalSetStateType(CEStateType::Land); }

	UFUNCTION(BlueprintCallable, Category = "State|Mode", meta = (DisplayName = "Set Dash"))
	void SetDash() { InternalSetStateType(CEStateType::Dash); }

	UFUNCTION(BlueprintCallable, Category = "State|Mode", meta = (DisplayName = "Set Cinematic"))
	void SetCinematic() { InternalSetStateType(CEStateType::Cinematic); }

	UFUNCTION(BlueprintCallable, Category = "State|Mode", meta = (DisplayName = "Set Groggy"))
	void SetGroggy() { InternalSetStateType(CEStateType::Groggy); }

	// 레거시 BP StateComponent::ChangeType 의 실제 상태 반영(Set* 도 공통 경로).
	UFUNCTION(BlueprintCallable, Category = "State|Change", meta = (DisplayName = "Change Type"))
	void ChangeType(CEStateType InNewType);

	// 레거시 BP 노드 이름 SetMode — ChangeType 과 동일.
	UFUNCTION(BlueprintCallable, Category = "State|Change", meta = (DisplayName = "Set Mode"))
	void SetMode(CEStateType InStateType);

	// 현재 타입을 Prev 에 복사만 한다(BP SaveType).
	UFUNCTION(BlueprintCallable, Category = "State|Change")
	void SaveType();

	// Type 과 Prev 를 스왑 후 델리게이트 브로드캐스트(BP LoadType).
	UFUNCTION(BlueprintCallable, Category = "State|Change")
	void LoadType();

private:
	void InternalSetStateType(CEStateType InType);

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

	// 블루프린트 StateComponent::IsRealRiding — 레거시 BP 는 CurInteractingActor 유효성, C++ 플래그는 보조.
	UFUNCTION(BlueprintPure, Category = "State")
	bool IsRealRiding() const;

	// 레거시 BP_Player RestoreStamina/Mana — 타입이 Riding 이면서 실제 상호작용/플래그로 탑승 중일 때 회복 허용.
	UFUNCTION(BlueprintPure, Category = "State")
	bool IsRidingRecoverContext() const;

	// 레거시 BP 의 Is Riding — CurType 만 검사(StateComponent 변수 Type 기준과 동일).
	UFUNCTION(BlueprintPure, Category = "State", meta = (DisplayName = "Is Riding"))
	bool IsRiding() const;

	// 현재 StateType 문자열 조회(GetEnumeratorNameAsString 근사).
	UFUNCTION(BlueprintPure, Category = "State|Mode")
	FString GetStringState() const;

	// 블루프린트 IsMoveable 분기표 — Idle / Equip / CombatHitted 만 true.
	UFUNCTION(BlueprintPure, Category = "State|Mode", meta = (DisplayName = "Is Moveable"))
	bool IsMoveable() const;

	UFUNCTION(BlueprintPure, Category = "State|Mode", meta = (DisplayName = "Is Equip"))
	bool IsEquip() const { return IsEquipMode(); }

	UFUNCTION(BlueprintPure, Category = "State|Mode", meta = (DisplayName = "Is Action"))
	bool IsAction() const { return IsActMode(); }

	// 레거시 BP 변수 Type 과의 비교(Is Hitted 상태). 피격 타입 IsHitted() 와 이름이 같아 주석과 DisplayName 로 구분한다.
	FORCEINLINE constexpr bool IsMovementHittedState() const { return CurType == CEStateType::CombatHitted; }
	UFUNCTION(BlueprintPure, Category = "State|Mode", meta = (DisplayName = "Is Hitted Movement State"))
	bool Blueprint_IsMovementHittedState() const { return IsMovementHittedState(); }

	UFUNCTION(BlueprintPure, Category = "State|Mode")
	bool IsFalling() const { return IsFallMode(); }

	UFUNCTION(BlueprintPure, Category = "State|Mode")
	bool IsDash() const { return CurType == CEStateType::Dash; }

	UFUNCTION(BlueprintPure, Category = "State|Mode")
	bool IsParkour() const { return CurType == CEStateType::Parkour; }

	UFUNCTION(BlueprintPure, Category = "State|Mode")
	bool IsFlying() const { return CurType == CEStateType::Flying; }

	UFUNCTION(BlueprintPure, Category = "State|Mode")
	bool IsHitAir() const { return CurType == CEStateType::HitAir; }

	UFUNCTION(BlueprintPure, Category = "State|Mode")
	bool IsDownFlying() const { return CurType == CEStateType::DownFlying; }

	UFUNCTION(BlueprintPure, Category = "State|Mode")
	bool IsDownLand() const { return CurType == CEStateType::Land; }

	UFUNCTION(BlueprintPure, Category = "State|Mode", meta = (DisplayName = "Is Rise"))
	bool IsRise() const { return IsRiseMode(); }

	UFUNCTION(BlueprintPure, Category = "State|Mode")
	bool IsGroggy() const { return CurType == CEStateType::Groggy; }

	UFUNCTION(BlueprintPure, Category = "State|Mode")
	bool IsCinematic() const { return CurType == CEStateType::Cinematic; }

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
	CEStateType PrevType = CEStateType::Idle;

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