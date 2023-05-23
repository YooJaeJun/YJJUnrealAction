#include "Weapons/Acts/CAct_Bow.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "Components/CStateComponent.h"

void UCAct_Bow::Act()
{
	CheckTrue(ActDatas.Num() < 1);
	CheckFalse(StateComp->IsIdleMode());

	Super::Act();

	ActDatas[0].Act(Owner.Get());
}

void UCAct_Bow::Begin_Act()
{
	Super::Begin_Act();
}

void UCAct_Bow::End_Act()
{
	Super::End_Act();
}

void UCAct_Bow::OnAttachmentBeginOverlap(ACCommonCharacter* InAttacker, AActor* InAttackCauser,
	ACCommonCharacter* InOther)
{
	Super::OnAttachmentBeginOverlap(InAttacker, InAttackCauser, InOther);
	CheckNull(InOther);

	for (ACCommonCharacter* hitted : Hitted)
		CheckTrue(hitted == InOther);

	Hitted.AddUnique(InOther);

	CheckTrue(HitDatas.Num() == 0);
	HitDatas[0].SendDamage(InAttacker, InAttackCauser, InOther);
}

void UCAct_Bow::OnAttachmentEndCollision()
{
	Super::OnAttachmentEndCollision();

	float angle = -1.0f;
	TWeakObjectPtr<ACCommonCharacter> candidate = nullptr;

	Hitted.Empty();
}
