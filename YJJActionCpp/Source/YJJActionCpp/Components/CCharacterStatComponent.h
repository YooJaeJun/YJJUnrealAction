#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CCharacterStatComponent.generated.h"

class UCGameInstance;
struct FCCharacterStat;

DECLARE_MULTICAST_DELEGATE(FHpIsZero);
DECLARE_MULTICAST_DELEGATE(FStaminaIsZero);
DECLARE_MULTICAST_DELEGATE(FManaIsZero);
DECLARE_MULTICAST_DELEGATE(FHpChanged);
DECLARE_MULTICAST_DELEGATE(FStaminaChanged);
DECLARE_MULTICAST_DELEGATE(FManaChanged);

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
	void SetNewLevel(const int32 InNewLevel, const float InRemainExp = 0.0f);
	void AddExp(const float InNewExp);
	void SetExp(const float InNewExp);
	void SetDamage(const float InNewDamage);
	void SetHp(const float InNewHp);
	void SetStamina(const float InNewStamina);
	void SetMana(const float InNewMana);

	float GetHpRatio() const;
	float GetCurHp() const;
	float GetMaxHp() const;
	float GetAttack() const;

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
	FHpIsZero OnHpIsZero;
	FStaminaIsZero OnStaminaIsZero;
	FManaIsZero OnManaIsZero;
	FHpChanged OnHpChanged;
	FStaminaChanged OnStaminaChanged;
	FManaChanged OnManaChanged;

private:
	FCCharacterStat* CurStat;
};
