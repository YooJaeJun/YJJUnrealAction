#include "Components/CCharacterStatComponent.h"
#include "Global.h"
#include "Commons/CGameInstance.h"
#include "Characters/CCommonCharacter.h"
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

	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCCharacterStatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCCharacterStatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SetNewLevel(1, 0.0f);
}

void UCCharacterStatComponent::SetNewLevel(const int32 InNewLevel, const float InRemainExp)
{
	const UCGameInstance* gameInst = Cast<UCGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	CheckNull(gameInst);

	CurStat = *(gameInst->GetStat(InNewLevel));
	CheckRefNull(CurStat, "CurStat doesn't exist.");

	CurLevel = InNewLevel;
	SetExp(InRemainExp);
	SetHp(CurStat.MaxHp);
	SetStamina(CurStat.MaxStamina);
	SetMana(CurStat.MaxMana);

	if (OnLevelChanged.IsBound())
		OnLevelChanged.Broadcast();
}

void UCCharacterStatComponent::AddExp(const float InNewExp)
{
	SetExp(CurExp + InNewExp);
}

void UCCharacterStatComponent::SetExp(const float InNewExp)
{
	CurExp = InNewExp;

	if (true == FMath::IsNearlyEqual(CurExp, CurStat.MaxExp) ||
		CurExp > CurStat.MaxExp)
		SetNewLevel(CurLevel + 1, CurExp - CurStat.MaxExp);

	if (OnExpChanged.IsBound())
		OnExpChanged.Broadcast();
}

void UCCharacterStatComponent::SetDamage(const float InNewDamage)
{
	SetHp(FMath::Clamp<float>(CurHp - InNewDamage, 0.0f, CurStat.MaxHp));
}

void UCCharacterStatComponent::SetStaminaDamage(const float InNewDamage)
{
	SetStamina(FMath::Clamp<float>(CurStamina - InNewDamage, 0.0f, CurStat.MaxStamina));
}

void UCCharacterStatComponent::SetManaDamage(const float InNewDamage)
{
	SetMana(FMath::Clamp<float>(CurMana - InNewDamage, 0.0f, CurStat.MaxMana));
}

void UCCharacterStatComponent::SetHp(const float InNewHp)
{
	CurHp = InNewHp;

	if (OnHpChanged.IsBound())
		OnHpChanged.Broadcast();

	if (CurHp < KINDA_SMALL_NUMBER)
	{
		CurHp = 0.0f;
		OnHpIsZero.Broadcast();
	}
	else if (CurHp > GetMaxHp())
		CurHp = GetMaxHp();
}

void UCCharacterStatComponent::SetStamina(const float InNewStamina)
{
	CurStamina = InNewStamina;

	if (OnStaminaChanged.IsBound())
		OnStaminaChanged.Broadcast();

	if (CurStamina < KINDA_SMALL_NUMBER)
	{
		CurStamina = 0.0f;
		OnStaminaIsZero.Broadcast();
	}
	else if (CurStamina > GetMaxStamina())
		CurStamina = GetMaxStamina();
}

void UCCharacterStatComponent::SetMana(const float InNewMana)
{
	CurMana = InNewMana;

	if (OnManaChanged.IsBound())
		OnManaChanged.Broadcast();

	if (CurMana < KINDA_SMALL_NUMBER)
	{
		CurMana = 0.0f;
		OnManaIsZero.Broadcast();
	}
	else if (CurMana > GetMaxMana())
		CurMana = GetMaxMana();
}

float UCCharacterStatComponent::GetRatio(const float InMaxValue, const float InCurValue) const
{
	if (InMaxValue < KINDA_SMALL_NUMBER)
		return 0.0f;
	else
		return InCurValue / InMaxValue;
}

float UCCharacterStatComponent::GetExpRatio() const
{
	return GetRatio(CurStat.MaxExp, CurExp);
}

float UCCharacterStatComponent::GetHpRatio() const
{
	return GetRatio(CurStat.MaxHp, CurHp);
}

float UCCharacterStatComponent::GetStaminaRatio() const
{
	return GetRatio(CurStat.MaxStamina, CurStamina);
}

float UCCharacterStatComponent::GetManaRatio() const
{
	return GetRatio(CurStat.MaxMana, CurMana);
}

void UCCharacterStatComponent::Damage(const float InAmount)
{
	SetHp(CurHp + (InAmount * -1.0f));
}
