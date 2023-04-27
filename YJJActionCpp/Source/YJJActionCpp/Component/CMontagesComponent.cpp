#include "Component/CMontagesComponent.h"
#include "Global.h"
#include "GameFramework/Character.h"

UCMontagesComponent::UCMontagesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCMontagesComponent::PlayBackStepMode()
{
	PlayAnimMontage(EStateType::BackStep);
}

void UCMontagesComponent::PlayAnimMontage(EStateType InType)
{
	CheckNull(Owner);

	FMontagesData* data = Datas[(uint8)InType];

	if (nullptr == data || nullptr == data->Montage)
	{
		GLog->Log(ELogVerbosity::Error, "No Montages Data");

		return;
	}

	Owner->PlayAnimMontage(data->Montage, data->PlayRate);
}