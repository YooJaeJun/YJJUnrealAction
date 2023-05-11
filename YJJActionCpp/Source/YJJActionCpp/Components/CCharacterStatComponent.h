#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CCharacterStatComponent.generated.h"

class UCGameInstance;
struct FCCharacterStat;

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

	float GetCurLevel() const;
	float GetExpRatio() const;
	float GetCurExp() const;
	float GetMaxExp() const;

	float GetHpRatio() const;
	UFUNCTION(BlueprintCallable)
		float GetCurHp() const;
	float GetMaxHp() const;

	float GetStaminaRatio() const;
	float GetCurStamina() const;
	float GetMaxStamina() const;

	float GetManaRatio() const;
	float GetCurMana() const;
	float GetMaxMana() const;

	float GetAttack() const;

	void Damage(const float InAmount);

public:
	FORCEINLINE float GetHealth() { return CurHp; }
	FORCEINLINE bool IsDead() { return CurHp <= 0.0f; }

private:
	UPROPERTY(EditInstanceOnly, Category = "Status", Meta = (AllowPrivateAccess = true))
		int32 CurLevel;

	UPROPERTY(EditInstanceOnly, Category = "Status", Meta = (AllowPrivateAccess = true))
		float CurExp;

	UPROPERTY(EditInstanceOnly, Category = "Status", Meta = (AllowPrivateAccess = true))
		float CurHp;

	UPROPERTY(EditInstanceOnly, Category = "Status", Meta = (AllowPrivateAccess = true))
		float CurStamina;

	UPROPERTY(EditInstanceOnly, Category = "Status", Meta = (AllowPrivateAccess = true))
		float CurMana;

	UPROPERTY(EditInstanceOnly, Category = "Status", Meta = (AllowPrivateAccess = true))
		bool EnoughStamina;

	UPROPERTY(EditInstanceOnly, Category = "Status", Meta = (AllowPrivateAccess = true))
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
	FCCharacterStat* CurStat;
	TWeakObjectPtr<ACCommonCharacter> Owner;
};
