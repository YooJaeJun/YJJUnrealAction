#pragma once
#include "CoreMinimal.h"
#include "Commons/CGameInstance.h"
#include "Components/ActorComponent.h"
#include "CCharacterStatComponent.generated.h"

class UCGameInstance;
struct FCCharacterStat;
class ACCommonCharacter;

DECLARE_MULTICAST_DELEGATE(FLevelChanged);
DECLARE_MULTICAST_DELEGATE(FExpChanged);
DECLARE_MULTICAST_DELEGATE(FHpChanged);
DECLARE_MULTICAST_DELEGATE(FStaminaChanged);
DECLARE_MULTICAST_DELEGATE(FManaChanged);
DECLARE_MULTICAST_DELEGATE(FHpIsZero);
DECLARE_MULTICAST_DELEGATE(FStaminaIsZero);
DECLARE_MULTICAST_DELEGATE(FManaIsZero);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCCharacterStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCCharacterStatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;

public:
	UFUNCTION(BlueprintCallable)
		void SetDamage(const float InNewDamage);

	UFUNCTION(BlueprintCallable)
		void SetStaminaDamage(const float InNewDamage);

	UFUNCTION(BlueprintCallable)
		void SetManaDamage(const float InNewDamage);

	UFUNCTION(BlueprintCallable)
		void AddExp(const float InNewExp);

public:
	void SetNewLevel(const int32 InNewLevel, const float InRemainExp);
	void SetExp(const float InNewExp);

	void SetHp(const float InNewHp);
	void SetStamina(const float InNewStamina);
	void SetMana(const float InNewMana);

	FORCEINLINE float GetCurLevel() const { return CurLevel; }

	float GetRatio(const float InMaxValue, const float InCurValue) const;
	float GetExpRatio() const;
	FORCEINLINE float GetCurExp() const { return CurExp; }
	FORCEINLINE float GetMaxExp() const { return CurStat.MaxExp; }
	float GetHpRatio() const;
	FORCEINLINE float GetCurHp() const { return CurHp; }
	FORCEINLINE float GetMaxHp() const { return CurStat.MaxHp; }
	float GetStaminaRatio() const;
	FORCEINLINE float GetCurStamina() const { return CurStamina; }
	FORCEINLINE float GetMaxStamina() const { return CurStat.MaxStamina; }
	float GetManaRatio() const;
	FORCEINLINE float GetCurMana() const { return CurMana; }
	FORCEINLINE float GetMaxMana() const { return CurStat.MaxMana; }
	FORCEINLINE float GetAttack() const { return CurStat.Attack; }
	FORCEINLINE float GetHealth() const { return CurHp; }
	FORCEINLINE bool IsDead() const { return CurHp <= 0.0f; }

	void Damage(const float InAmount);

public:
	FORCEINLINE void SetAttackRange(const float InAttackRange) { AttackRange = InAttackRange; }
	FORCEINLINE float GetAttackRange() const { return AttackRange; }

public:
	UPROPERTY(EditAnywhere)
		int32 CurLevel;

	UPROPERTY(EditAnywhere)
		float AttackRange = 100.0f;

private:
	UPROPERTY(EditInstanceOnly, Meta = (AllowPrivateAccess = true))
		float CurExp;

	UPROPERTY(EditInstanceOnly, Meta = (AllowPrivateAccess = true))
		float CurHp;

	UPROPERTY(EditInstanceOnly, Meta = (AllowPrivateAccess = true))
		float CurStamina;

	UPROPERTY(EditInstanceOnly, Meta = (AllowPrivateAccess = true))
		float CurMana;

	UPROPERTY(EditInstanceOnly, Meta = (AllowPrivateAccess = true))
		bool EnoughStamina;

	UPROPERTY(EditInstanceOnly, Meta = (AllowPrivateAccess = true))
		bool EnoughMana;

public:
	FLevelChanged OnLevelChanged;
	FExpChanged OnExpChanged;
	FHpChanged OnHpChanged;
	FStaminaChanged OnStaminaChanged;
	FManaChanged OnManaChanged;
	FHpIsZero OnHpIsZero;
	FStaminaIsZero OnStaminaIsZero;
	FManaIsZero OnManaIsZero;

private:
	FCCharacterStat CurStat;
	TWeakObjectPtr<ACCommonCharacter> Owner;
};
