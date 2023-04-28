#include "Component/CMontagesComponent.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "GameFramework/Character.h"

UCMontagesComponent::UCMontagesComponent()
{
}

void UCMontagesComponent::PlayBackStepMode()
{
	PlayAnimMontage(EStateType::BackStep);
}

void UCMontagesComponent::PlayAnimMontage(const EStateType InType)
{
	CheckNull(Owner);

	TSharedPtr<FMontagesData> data = Datas[(uint8)InType];

	if (nullptr == data || nullptr == data->Montage)
	{
		GLog->Log(ELogVerbosity::Error, "No Montages Data");

		return;
	}

	Owner->PlayAnimMontage(data->Montage, data->PlayRate);
}