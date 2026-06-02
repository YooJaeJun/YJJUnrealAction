#pragma once

#include "CoreMinimal.h"
#include "Commons/CEnums.h"
#include "Weapons/CSkillWeapon.h"
#include "Components/ActorComponent.h"
#include "CMagicComponent.generated.h"

// BP MagicComponent 의 OnMagicTypeChanged — 입력 순서 EMagicType InType 뒤 InPrevType.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMagicEquippedTypeChanged, CEMagicType, InType, CEMagicType, InPrevType);

// BP_Player 의 MagicComponent_C 포팅: Magic 액터 배열·형 타입 상태·변경 디스패치.
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YJJACTIONCPPUE5_API UCMagicComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCMagicComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// WeaponComponent 장착 흐름과 동기화 — 시각 상태·블루프린트 디스패치까지 한 번에 맞춘다.
	void SyncEquippedMagicAfterWeaponPipeline(CEMagicType CurrentMagic, CEMagicType PreviousMagic);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic", meta = (DisplayName = "Magic Classes", ToolTip = "배열 인덱스는 CEMagicType 의 정수 순서(Unarmed=0…)와 동일해야 한다. 비워 두면 해당 슬롯은 스폰하지 않는다."))
	TArray<TSubclassOf<ACMagic>> MagicClasses;

	// 블루프린트 VarName 의 type — 현재 선택 EMagicType. DisplayName 만 BP 와 통일했다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic", meta = (DisplayName = "Type"))
	CEMagicType MagicTypeSlot = CEMagicType::Unarmed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
	TArray<TObjectPtr<ACMagic>> Magics;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic", meta = (DisplayName = "Magic"))
	TObjectPtr<ACMagic> ActiveMagic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic")
	FName AttachSocketName = NAME_None;

	UPROPERTY(BlueprintAssignable, Category = "Magic")
	FOnMagicEquippedTypeChanged OnMagicTypeChanged;

	FORCEINLINE constexpr CEMagicType GetEquippedMagicSlot() const { return MagicTypeSlot; }

	// ---------------------------- BP MagicComponent: Mode ----------------------------

	UFUNCTION(BlueprintPure, Category = "Mode")
	bool IsUnarmed() const;

	UFUNCTION(BlueprintCallable, Category = "Mode")
	void SetUnarmed();

	UFUNCTION(BlueprintCallable, Category = "Mode")
	void SetWarp();

	UFUNCTION(BlueprintCallable, Category = "Mode")
	void SetAround();

	UFUNCTION(BlueprintCallable, Category = "Mode")
	void SetFireBall();

	UFUNCTION(BlueprintCallable, Category = "Mode")
	void SetBomb();

	UFUNCTION(BlueprintCallable, Category = "Mode")
	void SetYondu();

	// ---------------------------- BP MagicComponent: Utility ----------------------------

	UFUNCTION(BlueprintCallable, Category = "Utility")
	void SetMode(CEMagicType InType);

	UFUNCTION(BlueprintCallable, Category = "Utility")
	void ChangeType(CEMagicType InType);

	UFUNCTION(BlueprintCallable, Category = "Utility")
	void GetType(CEMagicType& OutMainType);

	// ---------- BP MagicComponent EventGraph — 현재 Magic 액터로 그대로 포워드 ----------

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void Begin_Equip();

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void End_Equip();

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void Begin_Unequip();

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void End_Unequip();

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void DoAction(CEAttackType InAttackType, int32 InSkillIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void Begin_DoAction(CEAttackType InAttackType);

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void End_DoAction(CEAttackType InAttackType);

private:
	void SpawnMagicActorsAndHideAll();
	void ApplyMagicActorsVisibilityFromSlot();
	static int32 SlotCountCeiling();

	// MagicTypeSlot 갱신 + 가시성 + OnMagicTypeChanged(InNext, InDelegatePrev)—Weapon 연동 시에는 외부에서 넘긴 Previous 를 쓴다.
	void CommitMagicTypeSlotApplyVisibilityMulticast(CEMagicType InNextMagicType, CEMagicType InDelegatePrevMagicType);
};
