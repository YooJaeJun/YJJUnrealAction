#include "Weapons/Acts/CAct_Bow.h"
#include "Global.h"
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
