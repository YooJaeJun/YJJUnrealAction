#include "Components/CCharacterStatComponent.h"
#include "Global.h"
#include "Game/CGameInstance.h"

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
}

void UCCharacterStatComponent::InitializeComponent()
{
	Super::InitializeComponent();
	SetNewLevel(1);
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
	{
		SetNewLevel(CurStat->MaxExp - CurExp);
	}
}

void UCCharacterStatComponent::SetDamage(const float InNewDamage)
{
	CheckNull(CurStat);
	SetHp(FMath::Clamp<float>(CurHp - InNewDamage, 0.0f, CurStat->MaxHp));
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

float UCCharacterStatComponent::GetAttack() const
{
	CheckNullResult(CurStat, 0.0f);

	return CurStat->Attack;
}
