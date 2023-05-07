#include "Components/CMontagesComponent.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "GameFramework/Character.h"

UCMontagesComponent::UCMontagesComponent()
{
}

void UCMontagesComponent::BeginPlay()
{
	Super::BeginPlay();

	if (nullptr == DataTable)
	{
		GLog->Log(ELogVerbosity::Error, "DataTable is not selected");

		return;
	}
	Owner = Cast<ACCommonCharacter>(GetOwner());

	TArray<FMontagesData*> datas;
	DataTable->GetAllRows<FMontagesData>("", datas);

	for (int32 i=0; i<static_cast<int32>(EStateType::Max); i++)
	{
		for (FMontagesData* data : datas)
		{
			if (static_cast<EStateType>(i) == data->StateType)
			{
				Datas[i] = *data;

				continue;
			}
		}//for(data)
	}//for(i)
}


void UCMontagesComponent::PlayAvoidAnim()
{
	PlayAnimMontage(EStateType::Avoid);
}

void UCMontagesComponent::PlayHitAnim()
{
	PlayAnimMontage(EStateType::Hit);
}

void UCMontagesComponent::PlayDeadAnim()
{
	PlayAnimMontage(EStateType::Dead);
}

void UCMontagesComponent::PlayAnimMontage(const EStateType InType)
{
	CheckNull(Owner);

	const FMontagesData data = Datas[(uint8)InType];

	if (nullptr == data.Montage)
	{
		GLog->Log(ELogVerbosity::Error, "No Montages Data");

		return;
	}

	Owner->PlayAnimMontage(data.Montage, data.PlayRate);
}
