#include "Components/CCharacterStatComponent.h"
#include "Global.h"
#include "Game/CGameInstance.h"
#include "Character/CCommonCharacter.h"
#include "Components/CStateComponent.h"

UCCharacterStatComponent::UCCharacterStatComponent()
{
	bWantsInitializeComponent = true;

	CurLevel = 1;
	CurExp = 100;
	CurHp = 200;
	CurStamina = 100;
	CurMana = 100;
	EnoughStamina = false;
	EnoughMana = false;
}

void UCCharacterStatComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCCharacterStatComponent::InitializeComponent()
{
	Super::InitializeComponent();
	SetNewLevel(1, 0.0f);
}

void UCCharacterStatComponent::SetNewLevel(const int32 InNewLevel, const float InRemainExp)
{
	UCGameInstance* gameInst = Cast<UCGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	CheckNull(gameInst);

	CurStat = gameInst->GetStat(InNewLevel);

	if (nullptr != CurStat)
	{
		CurLevel = InNewLevel;
		SetExp(InRemainExp);
		SetHp(CurStat->MaxHp);
		SetStamina(CurStat->MaxStamina);
		SetMana(CurStat->MaxMana);
		OnLevelChanged.Broadcast();
	}
	else
		CLog::Log("New Level data doesn't exist.");
}

void UCCharacterStatComponent::AddExp(const float InNewExp)
{
	SetExp(CurExp + InNewExp);
}

void UCCharacterStatComponent::SetExp(const float InNewExp)
{
	CurExp = InNewExp;

	if (true == FMath::IsNearlyEqual(CurExp, CurStat->MaxExp) ||
		CurExp > CurStat->MaxExp)
		SetNewLevel(CurLevel + 1, CurExp - CurStat->MaxExp);

	OnExpChanged.Broadcast();

	CLog::Print("Level : " + FString::FromInt(CurLevel), -1, 5, FColor::Cyan);
	CLog::Print("Exp : " + FString::FromInt((int)CurExp), -1, 5, FColor::Cyan);
}

void UCCharacterStatComponent::SetDamage(const float InNewDamage)
{
	CheckNull(CurStat);
	
	SetHp(FMath::Clamp<float>(CurHp - InNewDamage, 0.0f, CurStat->MaxHp));

	TWeakObjectPtr<UCStateComponent> state = 
		Cast<UCStateComponent>(Owner->GetComponentByClass(UCStateComponent::StaticClass()));

	state->SetHitMode();
}

void UCCharacterStatComponent::SetStaminaDamage(const float InNewDamage)
{
	CheckNull(CurStat);
	SetStamina(FMath::Clamp<float>(CurStamina - InNewDamage, 0.0f, CurStat->MaxStamina));
}

void UCCharacterStatComponent::SetManaDamage(const float InNewDamage)
{
	CheckNull(CurStat);
	SetMana(FMath::Clamp<float>(CurMana - InNewDamage, 0.0f, CurStat->MaxMana));
}

void UCCharacterStatComponent::SetHp(const float InNewHp)
{
	CurHp = InNewHp;
	OnHpChanged.Broadcast();

	if (CurHp < KINDA_SMALL_NUMBER)
	{
		CurHp = 0.0f;
		OnHpIsZero.Broadcast();
	}
}

void UCCharacterStatComponent::SetStamina(const float InNewStamina)
{
	CurStamina = InNewStamina;
	OnStaminaChanged.Broadcast();

	if (CurStamina < KINDA_SMALL_NUMBER)
	{
		CurStamina = 0.0f;
		OnStaminaIsZero.Broadcast();
	}
}

void UCCharacterStatComponent::SetMana(const float InNewMana)
{
	CurMana = InNewMana;
	OnManaChanged.Broadcast();

	if (CurMana < KINDA_SMALL_NUMBER)
	{
		CurMana = 0.0f;
		OnManaIsZero.Broadcast();
	}
}

float UCCharacterStatComponent::GetCurLevel() const
{
	CheckNullResult(CurStat, 0.0f);
	return CurLevel;
}

float UCCharacterStatComponent::GetExpRatio() const
{
	CheckNullResult(CurStat, 0.0f);

	if (CurStat->MaxExp < KINDA_SMALL_NUMBER)
		return 0.0f;
	else
		return CurExp / CurStat->MaxExp;
}

float UCCharacterStatComponent::GetCurExp() const
{
	CheckNullResult(CurStat, 0.0f);
	return CurExp;
}

float UCCharacterStatComponent::GetMaxExp() const
{
	CheckNullResult(CurStat, 0.0f);
	return CurStat->MaxExp;
}

float UCCharacterStatComponent::GetHpRatio() const
{
	CheckNullResult(CurStat, 0.0f);

	if (CurStat->MaxHp < KINDA_SMALL_NUMBER)
		return 0.0f;
	else
		return CurHp / CurStat->MaxHp;
}

float UCCharacterStatComponent::GetCurHp() const
{
	CheckNullResult(CurStat, 0.0f);
	return CurHp;
}

float UCCharacterStatComponent::GetMaxHp() const
{
	CheckNullResult(CurStat, 0.0f);
	return CurStat->MaxHp;
}

float UCCharacterStatComponent::GetStaminaRatio() const
{
	CheckNullResult(CurStat, 0.0f);

	if (CurStat->MaxStamina < KINDA_SMALL_NUMBER)
		return 0.0f;
	else
		return CurStamina / CurStat->MaxStamina;
}

float UCCharacterStatComponent::GetCurStamina() const
{
	CheckNullResult(CurStat, 0.0f);
	return CurStamina;
}

float UCCharacterStatComponent::GetMaxStamina() const
{
	CheckNullResult(CurStat, 0.0f);
	return CurStat->MaxStamina;
}

float UCCharacterStatComponent::GetManaRatio() const
{
	CheckNullResult(CurStat, 0.0f);

	if (CurStat->MaxMana < KINDA_SMALL_NUMBER)
		return 0.0f;
	else
		return CurMana / CurStat->MaxMana;
}

float UCCharacterStatComponent::GetCurMana() const
{
	CheckNullResult(CurStat, 0.0f);
	return CurMana;
}

float UCCharacterStatComponent::GetMaxMana() const
{
	CheckNullResult(CurStat, 0.0f);
	return CurStat->MaxMana;
}

float UCCharacterStatComponent::GetAttack() const
{
	CheckNullResult(CurStat, 0.0f);
	return CurStat->Attack;
}
