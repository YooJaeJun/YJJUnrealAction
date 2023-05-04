#include "Weapons/Acts/CAct_Combo.h"
#include "Global.h"
#include "GameFramework/Character.h"
#include "Component/CStateComponent.h"

void UCAct_Combo::Act()
{
	CheckTrue(ActDatas.Num() < 1);

	if (bEnable)
	{
		bEnable = false;
		bExist = true;

		return;
	}

	CheckFalse(State->IsIdleMode());

	Super::Act();
	ActDatas[Index].Act(Owner.Get());
}

void UCAct_Combo::Begin_Act()
{
	Super::Begin_Act();
	CheckFalse(bExist);

	bExist = false;
	ActDatas[++Index].Act(Owner.Get());
}

void UCAct_Combo::End_Act()
{
	Super::End_Act();

	Index = 0;
}
